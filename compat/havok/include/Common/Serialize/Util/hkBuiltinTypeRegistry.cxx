// Thin forwarding stub (Havok->Bullet compat shim, Phase A).
// Real Havok/hkBuiltinTypeRegistry.cxx registers every serializable Havok
// class's reflection metadata, used by the tagfile/packfile .hkx readers.
// This shim's hkpHavokSnapshot::load() is a Phase B stub that never
// actually deserializes class-by-class, so there is no reflection registry
// to populate -- empty on purpose. See docs/porting/havok-compat.md.
