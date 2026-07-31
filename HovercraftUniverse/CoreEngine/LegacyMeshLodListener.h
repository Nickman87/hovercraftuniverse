#ifndef LEGACYMESHLODLISTENER_H_
#define LEGACYMESHLODLISTENER_H_

// OgreMesh.h must be included before OgreMeshSerializer.h: the serializer
// header declares members of type SharedPtr<Mesh>/SharedPtr<DataStream> by
// value, which requires Ogre::SharedPtr's template definition (pulled in by
// OgreMesh.h's own #include "OgreSharedPtr.h") to already be complete --
// OgreMeshSerializer.h does not include it itself.
#include <OgreMesh.h>
#include <OgreMeshSerializer.h>

namespace HovUni {

/**
 * Ogre 14 API fix / port workaround (docs/porting/ogre-api-gap.md): strips
 * LOD levels from meshes loaded with the legacy [MeshSerializer_v1.41]
 * format, working around a distance-dependent rendering artifact on
 * SimpleTrack2's asteroids.
 *
 * Symptom: stable, geometry-shaped black patches appear on Asteroid01/
 * Asteroid02 at moderate camera distance and resolve correctly as the
 * camera approaches. Five other hypotheses were tried and eliminated
 * experimentally before this one: shadows (SHADOWTYPE_NONE changed
 * nothing), mipmaps (setDefaultNumMipmaps(0) changed nothing), far-plane
 * clipping (RaceCamera's far plane is 30000, nowhere near the artifact
 * range), material LOD (none of the shipped materials use it), and
 * z-fighting (the artifact is stable, not shimmering, so it isn't a
 * depth-precision race).
 *
 * Confirmed cause: mesh LOD. 17 of the 24 meshes loaded by SimpleTrack2
 * carry 3 LOD levels each, including Asteroid01.mesh and Asteroid02.mesh --
 * exactly the meshes showing the artifact. Ogre logs these meshes as using
 * "an old format [MeshSerializer_v1.41]" on load. A temporary experiment
 * that called Mesh::removeLodLevels() from the render loop made the
 * artifacts disappear within a second, verified by a human tester.
 *
 * Ogre 14's legacy read path gets two separate things wrong about these
 * meshes, and only the first was understood at first:
 *
 *   1. MeshLodUsage::value is left at 0 on every level while userValue
 *      holds the correctly authored switch distance (measured: userValue =
 *      0 / 200 / 400, value = 0 / 0 / 0 on every LOD-bearing mesh in
 *      SimpleTrack2). 'value' is what the LOD strategy compares against the
 *      camera each frame, so level selection was meaningless. The missing
 *      step is LodStrategy::transformUserValue().
 *
 *   2. The reduced levels' geometry itself is wrong.
 *
 * Re-deriving value from userValue via Mesh::setLodStrategy() fixed (1) and
 * appeared to fix the artifact. It did not -- it only moved it out of view.
 * The player spends nearly all their time inside Asteroid01/02's 200-unit
 * LOD 0 band, so those meshes' reduced levels are almost never displayed.
 * Planet01 -- the third asteroid, at (-1278, 281, -454), approached from
 * 700+ units away -- renders at LOD 2 for most of the approach, and still
 * showed the artifact with exactly the distance-dependent signature (2)
 * predicts. Discarding the reduced levels removes it there completely.
 *
 * So this discards the broken levels rather than trusting them: these
 * meshes are small by the standards of a modern GPU, so always rendering
 * full detail costs essentially nothing, and HovercraftUniverse/data/ must
 * not be rewritten to fix the LOD data on disk (the assets are recovered
 * byte-for-byte and their integrity is a project deliverable). If the LOD
 * levels are ever actually wanted back, regenerate them from LOD 0 with
 * Ogre 14's own MeshLodGenerator and re-apply the authored userValue
 * distances. See docs/porting/ogre-api-gap.md.
 *
 * Ogre 14 exposes exactly the hook needed: MeshSerializer::importMesh()
 * calls MeshSerializerListener::processMeshCompleted(Mesh*) once a mesh's
 * data has been fully read from disk (see OgreMeshSerializer.h), which is
 * the same hook the class comment there recommends for LOD generation --
 * here it's used for LOD removal instead. processMaterialName() and
 * processSkeletonName() are pure virtuals of the same interface but are
 * not needed for this fix, so both pass through unchanged.
 */
class LegacyMeshLodListener : public Ogre::MeshSerializerListener {
public:
	LegacyMeshLodListener();

	virtual ~LegacyMeshLodListener();

	/// @copydoc Ogre::MeshSerializerListener::processMaterialName
	void processMaterialName(Ogre::Mesh* mesh, Ogre::String* name) override;

	/// @copydoc Ogre::MeshSerializerListener::processSkeletonName
	void processSkeletonName(Ogre::Mesh* mesh, Ogre::String* name) override;

	/// @copydoc Ogre::MeshSerializerListener::processMeshCompleted
	void processMeshCompleted(Ogre::Mesh* mesh) override;

	/**
	 * Installs an instance of this listener on
	 * Ogre::MeshManager::getSingleton(). Must be called before any mesh is
	 * loaded (see Application::createRoot()), so this covers every mesh
	 * load, both at startup and per-race.
	 *
	 * Unlike DuplicateMaterialScriptCompilerListener::install(), there is no
	 * chaining here: MeshManager only ever has room for a single
	 * MeshSerializerListener (Ogre::MeshManager::setListener() simply
	 * overwrites the previous pointer), and nothing else in this codebase
	 * installs one, so there is nothing to chain to.
	 */
	static void install();
};

}

#endif
