#include "LegacyMeshLodListener.h"
#include <OgreMeshManager.h>
#include <OgreLogManager.h>
#include <OgreLodStrategyManager.h>

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
		// Diagnostic logging (docs/porting/ogre-api-gap.md): dump every LOD
		// level's distance value before stripping it. 'value' is what the
		// LOD strategy actually compares against the camera each frame;
		// 'userValue' is the value as originally authored, before
		// LodStrategy::transformUserValue() ran on it (e.g. squaring, for
		// the default distance strategy). Logging both lets us tell apart
		// "the authored LOD data is fine but Ogre 14 transformed it under
		// the wrong convention" (userValue sane, value nonsensical/absurdly
		// small) from "the LOD geometry/data itself is the problem"
		// (both columns look like sane, increasing distances).
		Ogre::StringStream diag;
		diag << "[LODDIAG] mesh '" << mesh->getName() << "' has " << lods
			<< " LOD levels (legacy format). Before:";
		for (unsigned short i = 0; i < lods; ++i) {
			const Ogre::MeshLodUsage& usage = mesh->getLodLevel(i);
			diag << " [" << i << ": value=" << usage.value
				<< " userValue=" << usage.userValue << "]";
		}

		// THE ACTUAL FIX (docs/porting/ogre-api-gap.md).
		//
		// Loading these MeshSerializer_v1.41 meshes leaves every LOD level's
		// 'value' at 0 while 'userValue' holds the correctly authored switch
		// distance (measured: userValue = 0 / 200 / 400, value = 0 / 0 / 0 on
		// every LOD-bearing mesh in SimpleTrack2). 'value' is what the LOD
		// strategy compares against the camera every frame, so with all of
		// them zero the level selection is meaningless and the renderer shows
		// heavily decimated geometry at distances where it should be showing
		// full detail -- the "black patches that resolve as you get closer"
		// artifact.
		//
		// The missing step is LodStrategy::transformUserValue(): the legacy
		// path populates userValue but never derives value from it.
		// Mesh::setLodStrategy() does exactly that derivation for every level
		// (level 0 gets the strategy's base value, the rest get
		// transformUserValue(userValue)), so re-applying the current strategy
		// reconstructs the data Ogre needs.
		//
		// This keeps the authored LOD levels working rather than discarding
		// them, and touches nothing on disk -- the assets under
		// HovercraftUniverse/data/ stay byte-for-byte as recovered.
		mesh->setLodStrategy(Ogre::LodStrategyManager::getSingleton().getDefaultStrategy());

		diag << " After:";
		for (unsigned short i = 0; i < mesh->getNumLodLevels(); ++i) {
			const Ogre::MeshLodUsage& usage = mesh->getLodLevel(i);
			diag << " [" << i << ": value=" << usage.value
				<< " userValue=" << usage.userValue << "]";
		}
		Ogre::LogManager::getSingleton().logMessage(diag.str());
	}
}

void LegacyMeshLodListener::install() {
	Ogre::MeshManager::getSingleton().setListener(new LegacyMeshLodListener());
}

}
