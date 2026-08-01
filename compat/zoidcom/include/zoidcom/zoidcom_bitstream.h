/****************************************
* zoidcom_bitstream.h
* bitstream class -- ZoidCom-compatible shim
*
* This one is implemented for real (not stubbed): a self-contained,
* self-consistent bit-level serialization buffer. See
* compat/zoidcom/src/BitStream.cpp and docs/porting/zoidcom-compat.md for
* fidelity notes -- in particular, bit layout is internally consistent but
* NOT wire-compatible with the original vendor's ZoidCom, which does not
* matter since no real ZoidCom binary exists anymore to interoperate with.
*****************************************/

#ifndef _ZCOM_BITSTREAM_H_
#define _ZCOM_BITSTREAM_H_

#include "zoidcom.h"

class ZCOM_API ZCom_BitStream
{
public:
  struct BitPos
  {
    zU16 bit;
    zU16 pos;
  };
protected:
  char*   m_ar;
  zU32    m_fill;
  BitPos  m_fillpos, m_readpos;
  zU16    m_size;
  zU16    m_maxfill;
  zU8     m_flags;
public:
  ZCom_BitStream( zU16 _maxfill = 64 );
  ZCom_BitStream( const ZCom_BitStream &_str );
  ~ZCom_BitStream();

  void Clear();
  ZCom_BitStream *Duplicate() const;

  void saveWriteState( BitPos &_pos ) const {_pos = m_fillpos;}
  void restoreWriteState( const BitPos &_pos ) {m_fillpos = _pos; m_fill = m_fillpos.pos * 8 + m_fillpos.bit; }
  void saveReadState( BitPos &_pos ) const {_pos = m_readpos;}
  void restoreReadState( const BitPos &_pos ) {m_readpos = _pos;}
  void resetReadState() {m_readpos.bit = 0;m_readpos.pos = 0;}

  void logReadState();
  void logWriteState();

  bool checkMax( zU32 _bits );
  inline bool checkFull() const {return ( m_fill / 8 > m_maxfill );}
  inline bool endOfStream() const {return m_readpos.pos * 8 + m_readpos.bit > m_fillpos.pos * 8 + m_fillpos.bit;}
  inline zU32 getBitCount() const {return m_fillpos.pos * 8 + m_fillpos.bit;}

  bool Serialize( char *_ptr, zU16 *_size, zU16 _max_size );
  bool Deserialize( char *_ptr, zU16 _size );
  zU16 getSizeHint( void );

  bool addInt( zU32 _data, zU8 _bits );
  zU32 getInt( zU8 _bits );
  void skipInt( zU8 _bits );

  bool addSignedInt( zS32 _data, zU8 _bits );
  zS32 getSignedInt( zU8 _bits );
  void skipSignedInt( zU8 _bits );

  bool addBool( bool _b );
  bool getBool();
  void skipBool();

  bool addFloat( zFloat _f, zU8 _mant_bits );
  zFloat getFloat( zU8 _mant_bits );
  void skipFloat( zU8 _mant_bits );

  bool addString( const char *_string );
  zU16 getStringSize();
  zU16 getStringLength();
  void getString( char *_buf, zU16 _maxsize );
  const char* getStringStatic();
  void skipString();

  bool addStringW( const wchar_t *_string );
  void getStringW( wchar_t *_buf, zU16 _maxsize );
  zU16 getStringWLength();
  const wchar_t* getStringWStatic();

  bool addBuffer( char *_buf, zU16 _size );
  zU16 getBuffer( char *_buf, zU16 _bytes );
  void skipBuffer(zU16 _bytes);
  zU16  getBufferMax( void );

  bool addBitStream( ZCom_BitStream *_stream, bool _allow_align = false );
  ZCom_BitStream *getBitStream( zU32 _bits, bool _allow_align = false );

  void skipBits(zU32 _amount);

  bool isEqual(const ZCom_BitStream& _other) const;

  void* operator new(size_t _size);
  void  operator delete(void *_p);

  ZCom_BitStream& operator=(const ZCom_BitStream &_str);

  // only for internal use (kept for source parity with the real API)
  char *getString();
protected:
  bool checkSize( zU32 _bits );
  inline void incPos( struct BitPos *_pos );
};

#endif
