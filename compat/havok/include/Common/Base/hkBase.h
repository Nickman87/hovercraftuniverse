// Thin forwarding header (Havok->Bullet compat shim, Phase A).
// Real path: Common/Base/hkBase.h
//
// This is the first Havok header AbstractHavokWorld.cpp includes, and real
// Havok's package headers gate physics availability behind
// USING_HAVOK_PHYSICS (AbstractHavokWorld.cpp does
// `#if !defined USING_HAVOK_PHYSICS #error ...`), so it's defined here,
// guaranteed to be visible before that check runs.
#pragma once
#define USING_HAVOK_PHYSICS 1
#include "havok_compat/HavokAll.h"
