// OgreCollisionProvider.h -- Phase B collision reconstruction (Ogre side).
// See docs/porting/phase-b-collision.md for the full design.
//
// Implements havok_compat::CollisionProvider (compat/havok/include/
// havok_compat/CollisionProvider.h) by rebuilding Bullet collision shapes
// from the same .scene + .mesh render data the game already ships, since
// the original Havok .hkx packfiles cannot be parsed (Havok 6.6 is dead,
// closed-source, and its binary format is undocumented at the wire level).
//
// Ownership: this module (compat/collision/**), not a port of anything
// under HovercraftUniverse/. Depends on Ogre + the vendored TinyXML under
// HovercraftUniverse/OgreMax/tinyxml (via hu_ogremax's include dir) -- this
// is exactly why the logic lives here and not in compat/havok, which must
// stay Ogre-free (see the design doc section 3.1).
#pragma once

#include "havok_compat/CollisionProvider.h"

namespace hu_collision {

// Registers itself as the active havok_compat::CollisionProvider on
// construction if `activate` is true (the normal case -- see
// registerDefaultProvider() below). Kept separate from construction so
// tests could construct one without touching the global registration.
class OgreCollisionProvider : public havok_compat::CollisionProvider {
public:
    OgreCollisionProvider() = default;
    ~OgreCollisionProvider() override = default;

    bool build(const std::string& hkxPath, std::vector<havok_compat::ReconstructedBody>& out) override;
};

// Constructs a process-lifetime OgreCollisionProvider and registers it via
// havok_compat::setCollisionProvider(). Call once, after Ogre resource
// groups exist (addResourceLocation has been called for at least the
// levels/hovercraft directories) but before any track/hovercraft load can
// race it -- see HUDedicatedServer::run(), the single game-source call site
// for this (docs/porting/phase-b-collision.md section 3.1's "one permitted
// game-source change").
void registerDefaultProvider();

}
