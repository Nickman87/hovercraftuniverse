// ZoidCom-compatible shim: ZCom_BitStream.
//
// Real, from-scratch bit-level serialization buffer. It is internally
// self-consistent (whatever addX() writes, the matching getX() reads back
// correctly) but is NOT bit-for-bit wire-compatible with the original
// vendor's ZoidCom encoding. That does not matter in practice: there is no
// surviving real-ZoidCom binary left to interoperate with (that's the
// entire reason this shim exists), and both ends of every connection in
// this codebase use this same implementation. See
// docs/porting/zoidcom-compat.md for more on this design choice.
//
// Phase B fix: addBitStream() previously wrote its own redundant embedded
// bit-count prefix in addition to copying the payload bits. That silently
// corrupted every caller using the calling convention getBitStream()'s
// signature actually implies (manually transmit/read the bit count, then
// call getBitStream(bits)) -- which is exactly what the game's own
// InitEvent::write()/read() does. See addBitStream()'s comment below for
// the full explanation. Found while implementing Phase B node-to-node
// event delivery (docs/porting/phase-b-replication.md step 2), which nests
// bitstreams the same way.
#include "zoidcom_shim_internal.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>

namespace {

const zU16 kStaticStringBufSize = 2048;
const zU16 kStaticWStringBufSize = 1024;

thread_local char tls_static_str_buf[kStaticStringBufSize];
thread_local wchar_t tls_static_wstr_buf[kStaticWStringBufSize];

inline zU32 bitsToBytes(zU32 bits) { return (bits + 7) / 8; }

} // namespace

ZCom_BitStream::ZCom_BitStream(zU16 _maxfill) {
    m_maxfill = _maxfill ? _maxfill : 64;
    m_size = m_maxfill;
    m_ar = (char*) zshim::zalloc(m_size);
    memset(m_ar, 0, m_size);
    m_fill = 0;
    m_fillpos.bit = 0; m_fillpos.pos = 0;
    m_readpos.bit = 0; m_readpos.pos = 0;
    m_flags = 0;
}

ZCom_BitStream::ZCom_BitStream(const ZCom_BitStream& _str) {
    m_size = _str.m_size;
    m_ar = (char*) zshim::zalloc(m_size ? m_size : 1);
    if (m_size) memcpy(m_ar, _str.m_ar, m_size);
    m_fill = _str.m_fill;
    m_fillpos = _str.m_fillpos;
    m_readpos = _str.m_readpos;
    m_maxfill = _str.m_maxfill;
    m_flags = _str.m_flags;
}

ZCom_BitStream::~ZCom_BitStream() {
    zshim::zfree(m_ar);
}

void ZCom_BitStream::Clear() {
    memset(m_ar, 0, m_size);
    m_fill = 0;
    m_fillpos.bit = 0; m_fillpos.pos = 0;
    m_readpos.bit = 0; m_readpos.pos = 0;
}

ZCom_BitStream* ZCom_BitStream::Duplicate() const {
    return new ZCom_BitStream(*this);
}

void ZCom_BitStream::logReadState() {
    zshim::logf("ZCom_BitStream read state: byte %u bit %u", (unsigned) m_readpos.pos, (unsigned) m_readpos.bit);
}

void ZCom_BitStream::logWriteState() {
    zshim::logf("ZCom_BitStream write state: byte %u bit %u", (unsigned) m_fillpos.pos, (unsigned) m_fillpos.bit);
}

bool ZCom_BitStream::checkSize(zU32 _bits) {
    zU32 needed_bytes = bitsToBytes(m_fillpos.pos * 8 + m_fillpos.bit + _bits);
    if (needed_bytes <= m_size) return true;

    zU16 new_size = m_size ? m_size : 16;
    while (new_size < needed_bytes) {
        new_size = (zU16) std::min<zU32>(65535u, (zU32) new_size * 2);
        if (new_size == m_size) { // overflowed zU16, can't grow further
            return false;
        }
    }
    char* new_ar = (char*) zshim::zalloc(new_size);
    memset(new_ar, 0, new_size);
    memcpy(new_ar, m_ar, m_size);
    zshim::zfree(m_ar);
    m_ar = new_ar;
    m_size = new_size;
    return true;
}

bool ZCom_BitStream::checkMax(zU32 _bits) {
    return checkSize(_bits);
}

void ZCom_BitStream::incPos(BitPos* _pos) {
    _pos->bit++;
    if (_pos->bit == 8) {
        _pos->bit = 0;
        _pos->pos++;
    }
}

// --- bit-level primitives (LSB-first within each written value) ---------

static void writeBit(ZCom_BitStream::BitPos& pos, char* ar, bool bit) {
    unsigned char mask = (unsigned char) (1u << pos.bit);
    if (bit) ar[pos.pos] |= (char) mask;
    else ar[pos.pos] &= (char) ~mask;
}

static bool readBit(const ZCom_BitStream::BitPos& pos, const char* ar) {
    unsigned char mask = (unsigned char) (1u << pos.bit);
    return (ar[pos.pos] & (char) mask) != 0;
}

bool ZCom_BitStream::addInt(zU32 _data, zU8 _bits) {
    if (_bits == 0 || _bits > 32) return false;
    if (!checkSize(_bits)) return false;
    for (zU8 i = 0; i < _bits; i++) {
        writeBit(m_fillpos, m_ar, ((_data >> i) & 1u) != 0);
        incPos(&m_fillpos);
    }
    m_fill = m_fillpos.pos * 8 + m_fillpos.bit;
    return true;
}

zU32 ZCom_BitStream::getInt(zU8 _bits) {
    zU32 result = 0;
    if (_bits == 0 || _bits > 32) return 0;
    for (zU8 i = 0; i < _bits; i++) {
        if (endOfStream()) break;
        if (readBit(m_readpos, m_ar)) result |= (1u << i);
        incPos(&m_readpos);
    }
    return result;
}

void ZCom_BitStream::skipInt(zU8 _bits) {
    for (zU8 i = 0; i < _bits; i++) {
        if (endOfStream()) break;
        incPos(&m_readpos);
    }
}

bool ZCom_BitStream::addSignedInt(zS32 _data, zU8 _bits) {
    bool negative = _data < 0;
    zU32 mag = negative ? (zU32) (-(zS64) _data) : (zU32) _data;
    if (!addBool(negative)) return false;
    return addInt(mag, _bits);
}

zS32 ZCom_BitStream::getSignedInt(zU8 _bits) {
    bool negative = getBool();
    zU32 mag = getInt(_bits);
    return negative ? -(zS32) mag : (zS32) mag;
}

void ZCom_BitStream::skipSignedInt(zU8 _bits) {
    skipBool();
    skipInt(_bits);
}

bool ZCom_BitStream::addBool(bool _b) {
    if (!checkSize(1)) return false;
    writeBit(m_fillpos, m_ar, _b);
    incPos(&m_fillpos);
    m_fill = m_fillpos.pos * 8 + m_fillpos.bit;
    return true;
}

bool ZCom_BitStream::getBool() {
    if (endOfStream()) return false;
    bool b = readBit(m_readpos, m_ar);
    incPos(&m_readpos);
    return b;
}

void ZCom_BitStream::skipBool() {
    if (!endOfStream()) incPos(&m_readpos);
}

// Float encoding: sign bit + 8 exponent bits + truncated mantissa (top
// _mant_bits of the 23 IEEE-754 mantissa bits). This mirrors the real
// ZoidCom docs' description of "9 extra bits for exponent/sign plus the
// mantissa bits you ask for", including the same kind of precision loss
// for small mantissa counts -- see docs/porting/zoidcom-compat.md.
bool ZCom_BitStream::addFloat(zFloat _f, zU8 _mant_bits) {
    if (_mant_bits > 23) _mant_bits = 23;
    zU32 bits;
    memcpy(&bits, &_f, sizeof(bits));
    bool sign = (bits & 0x80000000u) != 0;
    zU32 exponent = (bits >> 23) & 0xFFu;
    zU32 mantissa = bits & 0x7FFFFFu;
    zU32 truncated = _mant_bits == 0 ? 0 : (mantissa >> (23 - _mant_bits));

    if (!addBool(sign)) return false;
    if (!addInt(exponent, 8)) return false;
    if (_mant_bits > 0) {
        if (!addInt(truncated, _mant_bits)) return false;
    }
    return true;
}

zFloat ZCom_BitStream::getFloat(zU8 _mant_bits) {
    if (_mant_bits > 23) _mant_bits = 23;
    bool sign = getBool();
    zU32 exponent = getInt(8);
    zU32 truncated = _mant_bits > 0 ? getInt(_mant_bits) : 0;
    zU32 mantissa = _mant_bits > 0 ? (truncated << (23 - _mant_bits)) : 0;

    zU32 bits = (sign ? 0x80000000u : 0u) | (exponent << 23) | mantissa;
    zFloat f;
    memcpy(&f, &bits, sizeof(f));
    return f;
}

void ZCom_BitStream::skipFloat(zU8 _mant_bits) {
    if (_mant_bits > 23) _mant_bits = 23;
    skipBool();
    skipInt(8);
    if (_mant_bits > 0) skipInt(_mant_bits);
}

bool ZCom_BitStream::addString(const char* _string) {
    zU16 len = _string ? (zU16) strlen(_string) : 0;
    if (!addInt(len, 16)) return false;
    for (zU16 i = 0; i < len; i++) {
        if (!addInt((zU8) _string[i], 8)) return false;
    }
    return true;
}

zU16 ZCom_BitStream::getStringSize() {
    BitPos saved;
    saveReadState(saved);
    zU16 len = (zU16) getInt(16);
    restoreReadState(saved);
    return len;
}

zU16 ZCom_BitStream::getStringLength() {
    return getStringSize();
}

void ZCom_BitStream::getString(char* _buf, zU16 _maxsize) {
    zU16 len = (zU16) getInt(16);
    zU16 to_copy = (len < _maxsize - 1) ? len : (zU16) (_maxsize > 0 ? _maxsize - 1 : 0);
    zU16 i = 0;
    for (; i < len; i++) {
        zU8 c = (zU8) getInt(8);
        if (i < to_copy) _buf[i] = (char) c;
    }
    if (_maxsize > 0) _buf[to_copy] = '\0';
}

const char* ZCom_BitStream::getStringStatic() {
    getString(tls_static_str_buf, kStaticStringBufSize);
    return tls_static_str_buf;
}

void ZCom_BitStream::skipString() {
    zU16 len = (zU16) getInt(16);
    for (zU16 i = 0; i < len; i++) skipInt(8);
}

bool ZCom_BitStream::addStringW(const wchar_t* _string) {
    zU16 len = 0;
    if (_string) { while (_string[len] != 0) len++; }
    if (!addInt(len, 16)) return false;
    for (zU16 i = 0; i < len; i++) {
        if (!addInt((zU32) _string[i], 32)) return false;
    }
    return true;
}

void ZCom_BitStream::getStringW(wchar_t* _buf, zU16 _maxsize) {
    zU16 len = (zU16) getInt(16);
    zU16 to_copy = (len < _maxsize - 1) ? len : (zU16) (_maxsize > 0 ? _maxsize - 1 : 0);
    for (zU16 i = 0; i < len; i++) {
        wchar_t c = (wchar_t) getInt(32);
        if (i < to_copy) _buf[i] = c;
    }
    if (_maxsize > 0) _buf[to_copy] = 0;
}

zU16 ZCom_BitStream::getStringWLength() {
    BitPos saved;
    saveReadState(saved);
    zU16 len = (zU16) getInt(16);
    restoreReadState(saved);
    return len;
}

const wchar_t* ZCom_BitStream::getStringWStatic() {
    getStringW(tls_static_wstr_buf, kStaticWStringBufSize);
    return tls_static_wstr_buf;
}

bool ZCom_BitStream::addBuffer(char* _buf, zU16 _size) {
    if (!addInt(_size, 16)) return false;
    for (zU16 i = 0; i < _size; i++) {
        if (!addInt((zU8) _buf[i], 8)) return false;
    }
    return true;
}

zU16 ZCom_BitStream::getBuffer(char* _buf, zU16 _bytes) {
    zU16 size = (zU16) getInt(16);
    zU16 to_read = std::min(size, _bytes);
    for (zU16 i = 0; i < size; i++) {
        zU8 b = (zU8) getInt(8);
        if (i < to_read) _buf[i] = (char) b;
    }
    return to_read;
}

void ZCom_BitStream::skipBuffer(zU16 _bytes) {
    zU16 size = (zU16) getInt(16);
    for (zU16 i = 0; i < size; i++) skipInt(8);
    (void) _bytes;
}

zU16 ZCom_BitStream::getBufferMax(void) {
    return getStringSize();
}

bool ZCom_BitStream::addBitStream(ZCom_BitStream* _stream, bool _allow_align) {
    // NOTE (Phase B fix): this used to also addInt(bits, 32) a redundant
    // length prefix of its own here. That desyncs the read cursor for
    // every caller that follows the calling convention getBitStream()'s
    // own signature implies -- and that the game's own code already uses,
    // e.g. HovercraftUniverse/HovercraftUniverse/InitEvent.cpp:
    //   write(): stream->addInt(mStream->getBitCount(), 32);
    //            stream->addBitStream(mStream, true);
    //   read():  zU32 bits = stream->getInt(32);
    //            mStream = stream->getBitStream(bits, true);
    // getBitStream() takes an explicit _bits parameter (matching the real
    // ZoidCom signature) specifically because the caller is expected to
    // communicate/reconstruct the bit count itself, as InitEvent does. With
    // the old self-embedded prefix, that manual getInt(32) would consume
    // the *caller's* length field while leaving this function's own
    // (redundant) embedded length sitting unconsumed in the stream,
    // corrupting every subsequent read. Fixed by writing only the raw
    // payload bits here, matching getBitStream()'s contract exactly.
    if (!_stream) return false;
    zU32 bits = _stream->getBitCount() - (_stream->m_readpos.pos * 8 + _stream->m_readpos.bit);

    ZCom_BitStream::BitPos saved_read = _stream->m_readpos;
    for (zU32 i = 0; i < bits; i++) {
        bool b = _stream->getBool();
        addBool(b);
    }
    (void) _allow_align;
    _stream->m_readpos = saved_read; // don't disturb caller's stream
    return true;
}

ZCom_BitStream* ZCom_BitStream::getBitStream(zU32 _bits, bool _allow_align) {
    (void) _allow_align;
    ZCom_BitStream* out = new ZCom_BitStream((zU16) bitsToBytes(_bits) + 1);
    for (zU32 i = 0; i < _bits; i++) {
        if (endOfStream()) break;
        out->addBool(getBool());
    }
    return out;
}

void ZCom_BitStream::skipBits(zU32 _amount) {
    for (zU32 i = 0; i < _amount; i++) {
        if (endOfStream()) break;
        incPos(&m_readpos);
    }
}

bool ZCom_BitStream::isEqual(const ZCom_BitStream& _other) const {
    if (m_fill != _other.m_fill) return false;
    zU32 bytes = bitsToBytes(m_fill);
    return bytes == 0 || memcmp(m_ar, _other.m_ar, bytes) == 0;
}

bool ZCom_BitStream::Serialize(char* _ptr, zU16* _size, zU16 _max_size) {
    zU16 bytes = (zU16) bitsToBytes(m_fillpos.pos * 8 + m_fillpos.bit);
    if (bytes > _max_size) return false;
    memcpy(_ptr, m_ar, bytes);
    if (_size) *_size = bytes;
    return true;
}

bool ZCom_BitStream::Deserialize(char* _ptr, zU16 _size) {
    if (!checkSize(_size * 8)) return false;
    memcpy(m_ar, _ptr, _size);
    m_fillpos.pos = _size;
    m_fillpos.bit = 0;
    m_fill = _size * 8;
    m_readpos.pos = 0;
    m_readpos.bit = 0;
    return true;
}

zU16 ZCom_BitStream::getSizeHint(void) {
    return (zU16) bitsToBytes(m_fillpos.pos * 8 + m_fillpos.bit);
}

void* ZCom_BitStream::operator new(size_t _size) { return zshim::zalloc(_size); }
void ZCom_BitStream::operator delete(void* _p) { zshim::zfree(_p); }

ZCom_BitStream& ZCom_BitStream::operator=(const ZCom_BitStream& _str) {
    if (this == &_str) return *this;
    zshim::zfree(m_ar);
    m_size = _str.m_size;
    m_ar = (char*) zshim::zalloc(m_size ? m_size : 1);
    if (m_size) memcpy(m_ar, _str.m_ar, m_size);
    m_fill = _str.m_fill;
    m_fillpos = _str.m_fillpos;
    m_readpos = _str.m_readpos;
    m_maxfill = _str.m_maxfill;
    m_flags = _str.m_flags;
    return *this;
}

char* ZCom_BitStream::getString() {
    // This is the overload the game actually uses everywhere it reads a
    // string off the wire -- NOT getStringStatic(). It was previously a stub
    // that returned the (stale, usually empty) TLS buffer without consuming
    // anything from the stream, annotated "not used by game code (verified:
    // no callers found)". That verification was simply wrong: there are nine
    // call sites, including
    //
    //   CoreEngine/Entity.cpp:43,57      every entity's name + ogre entity name
    //   HovercraftUniverse/RaceState.cpp:112   the track filename (announce data)
    //   HovercraftUniverse/PlayerSettings.cpp:99   the player name
    //   HovercraftUniverse/Hovercraft.cpp:25       hovercraft display name
    //   Networking/TextEvent.cpp, NotifyEvent.cpp  all chat text
    //
    // Because it read nothing, it also left the read cursor parked before the
    // string, desyncing every subsequent field in the same stream -- so the
    // damage was never limited to the string itself.
    //
    // Symptoms this caused: an empty track filename, so the client built
    // resource locations like "levels//textures" and then threw
    // RuntimeAssertionException(!resourceName.empty()) opening the scene; and
    // a blank player name and hovercraft selection in the lobby.
    getString(tls_static_str_buf, kStaticStringBufSize);
    return (char*) tls_static_str_buf;
}
