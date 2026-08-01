#include "LegacyMeshLodListener.h"
#include <OgreMeshManager.h>
#include <OgreLogManager.h>

namespace HovUni {

LegacyMeshLodListener::LegacyMeshLodListener() {
}

LegacyMeshLodListener::~LegacyMeshLodListener() {
}

void LegacyMeshLodListener::processMaterialName(Ogre::Mesh* mesh, Ogre::String* name) {
	// Not needed for this fix -- pass through unchanged.
}

void LegacyMeshLodListener::processSkeletonName(Ogre::Mesh* mesh, Ogre::String* name) {
	// Not needed for this fix -- pass through unchanged.
}

void LegacyMeshLodListener::processMeshCompleted(Ogre::Mesh* mesh) {
	const unsigned short lods = mesh->getNumLodLevels();
	if (lods > 1) {
		// Ogre 14 cannot read this game's legacy LOD data correctly
		// (docs/porting/ogre-api-gap.md). Every LOD-bearing mesh here is
		// MeshSerializer_v1.41, and Ogre 14's legacy read path gets two
		// things wrong about it:
		//
		//  1. MeshLodUsage::value is left at 0 on every level while
		//     userValue holds the correctly authored switch distance
		//     (measured: userValue = 0 / 200 / 400, value = 0 / 0 / 0 on
		//     every LOD-bearing mesh in SimpleTrack2). 'value' is what the
		//     LOD strategy compares against the camera each frame, so the
		//     level selection was meaningless. The missing step is
		//     LodStrategy::transformUserValue().
		//
		//  2. The reduced levels' geometry itself is wrong.
		//
		// Only (1) was understood at first, and re-deriving value from
		// userValue via setLodStrategy() appeared to fix the "black patches
		// that resolve as you get closer" artifact. It did not -- it only
		// moved it out of view. Asteroid01/02 are where the player spends
		// nearly all their time, comfortably inside the 200-unit LOD 0 band,
		// so their reduced levels are almost never displayed. Planet01, the
		// third asteroid, sits at (-1278, 281, -454) and is approached from
		// 700+ units away, so it renders at LOD 2 for most of the approach --
		// and the artifact was still plainly there, with exactly the
		// distance-dependent signature (2) predicts: fixed in shape, resolving
		// as the camera closes and crosses into LOD 1 then 0.
		//
		// Verified by discarding the reduced levels entirely, which removes
		// the artifact on Planet01 completely.
		//
		// So: drop the broken levels and always draw full detail. These are
		// 2010-era meshes on modern hardware, where the LOD levels bought
		// nothing measurable anyway. This touches nothing on disk -- the
		// assets under HovercraftUniverse/data/ stay byte-for-byte as
		// recovered, and the authored LOD data is simply ignored at load.
		//
		// A refinement, if the LOD levels are ever actually wanted back:
		// regenerate them from LOD 0 with Ogre 14's own MeshLodGenerator and
		// re-apply the authored userValue distances, rather than trusting
		// what the legacy deserializer produced.
		mesh->removeLodLevels();

		Ogre::LogManager::getSingleton().logMessage(
			"[LegacyMeshLod] mesh '" + mesh->getName() + "': discarded "
			+ Ogre::StringConverter::toString(lods - 1)
			+ " legacy LOD level(s), drawing full detail");
	}
}

void LegacyMeshLodListener::install() {
	Ogre::MeshManager::getSingleton().setListener(new LegacyMeshLodListener());
}

}
