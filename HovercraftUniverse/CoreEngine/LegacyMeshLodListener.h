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
 * This is a deliberate port decision, not a root-cause fix: these meshes
 * are small by the standards of a modern GPU, so discarding their LOD
 * levels and always rendering full detail costs essentially nothing, and
 * HovercraftUniverse/data/ must not be rewritten to fix the LOD data on
 * disk (the assets are recovered byte-for-byte and their integrity is a
 * project deliverable). Whether the LOD *data* itself is wrong, or Ogre 14
 * is misinterpreting old-format LOD distance values (e.g. reading them
 * under the wrong squared-vs-plain-distance convention and switching to a
 * heavily decimated level far too early) is an open question -- see the
 * [LODDIAG] log lines this listener emits and the discussion in
 * docs/porting/ogre-api-gap.md.
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
