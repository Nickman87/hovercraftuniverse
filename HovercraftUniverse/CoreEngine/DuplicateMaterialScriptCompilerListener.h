#ifndef DUPLICATEMATERIALSCRIPTCOMPILERLISTENER_H_
#define DUPLICATEMATERIALSCRIPTCOMPILERLISTENER_H_

#include <OgreScriptCompiler.h>
#include <set>

namespace HovUni {

/**
 * Ogre 14 API fix (docs/porting/ogre-api-gap.md): restores Ogre 1.7's
 * "first definition wins" handling of duplicate material names in
 * .material scripts.
 *
 * The shipped .material scripts genuinely redefine the same material name
 * more than once -- both within a single file (e.g. Junkyard.material's
 * "Wing" appears twice) and across two files parsed into the same resource
 * group (e.g. Junkyard.material and Junkyard2.material both define "Wing").
 * These are original, unmodified assets (see docs/porting -- the data
 * files are a fixed point and are not to be touched).
 *
 * Ogre 1.7's script compiler tolerated this: on a redefinition it logged a
 * duplicate-resource error and skipped the redefinition entirely, so the
 * first definition silently won. Ogre 14 removed that guard --
 * MaterialTranslator::translate() (OgreScriptTranslator.cpp) now calls
 * MaterialManager::create() unconditionally for every "material" block,
 * which throws Ogre::ItemIdentityException the second time a given name
 * is created. That exception was reaching main.cpp's top-level catch and
 * putting up a modal MessageBox, permanently blocking the client.
 *
 * Ogre 14 exposes exactly the hook needed to intervene:
 * MaterialTranslator fires a CreateMaterialScriptCompilerEvent through
 * ScriptCompiler::_fireEvent() before calling
 * MaterialManager::getSingleton().create(); if a registered
 * ScriptCompilerListener::handleEvent() returns true and writes a
 * Material* through retval, Ogre uses that material instead of calling
 * create() itself (see OgreScriptTranslator.cpp, MaterialTranslator::
 * translate()).
 *
 * Simply handing back the *existing* material for a duplicate name would
 * not reproduce Ogre 1.7's behaviour: MaterialTranslator unconditionally
 * parses the block's contents into whatever Material* it receives (and
 * even calls removeAllTechniques() on it first), so that would let the
 * duplicate block silently overwrite the original -- i.e.
 * *last*-definition-wins, the opposite of the original engine. Instead,
 * on a duplicate name this listener hands back a uniquely-named, throwaway
 * scratch Material for the duplicate block to harmlessly clobber, leaving
 * the real, first-defined material completely untouched. Nothing else
 * ever references the scratch material by name, so it is inert.
 *
 * Resource names are effectively global in Ogre, so "does this name
 * already exist" is checked across all resource groups (via
 * ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME), not just the
 * resource group the current script is being parsed into -- this is what
 * makes the cross-file case (e.g. Bavaraf.material vs "Kopie van
 * Bavaraf.material", both parsed into the same group) work the same way
 * as the same-file case.
 *
 * Each suppressed duplicate is logged once per material name (at a
 * visible log level, naming the offending file) so this fix is never
 * silent -- it should read as "restoring Ogre 1.7 behaviour", not as an
 * error being swallowed.
 *
 * A full scan of HovercraftUniverse/data turned up no duplicate
 * vertex_program/fragment_program/geometry_program, particle_system, or
 * compositor names -- only materials collide -- so only
 * CreateMaterialScriptCompilerEvent is handled here. If that ever
 * changes, CreateGpuProgramScriptCompilerEvent,
 * CreateGpuSharedParametersScriptCompilerEvent,
 * CreateParticleSystemScriptCompilerEvent and
 * CreateCompositorScriptCompilerEvent would need the same treatment.
 */
class DuplicateMaterialScriptCompilerListener : public Ogre::ScriptCompilerListener {
public:
	/**
	 * @param chainedListener a previously-installed listener (may be null)
	 *        to forward events this listener doesn't itself handle to, so
	 *        installing this listener never silently clobbers another
	 *        one's behaviour.
	 */
	explicit DuplicateMaterialScriptCompilerListener(Ogre::ScriptCompilerListener* chainedListener);

	virtual ~DuplicateMaterialScriptCompilerListener();

	/// @copydoc Ogre::ScriptCompilerListener::handleEvent
	bool handleEvent(Ogre::ScriptCompiler* compiler, Ogre::ScriptCompilerEvent* evt, void* retval) override;

	/**
	 * Installs an instance of this listener on
	 * Ogre::ScriptCompilerManager, chaining to whatever listener (if any)
	 * is already installed there. Must be called before any resource
	 * group is initialised (see Application::createRoot()), so this
	 * covers the resource groups Ogre parses at startup as well as the
	 * per-race "Track" group.
	 */
	static void install();

private:
	/** A previously-installed listener to forward unhandled events to (may be null). */
	Ogre::ScriptCompilerListener* mChainedListener;

	/** Material names for which a duplicate-definition log message has already been emitted. */
	std::set<Ogre::String> mAlreadyLoggedNames;

	/** Monotonically increasing suffix, used to keep scratch material names unique. */
	unsigned int mNextScratchId;
};

}

#endif
