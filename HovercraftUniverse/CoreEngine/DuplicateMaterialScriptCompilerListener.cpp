#include "DuplicateMaterialScriptCompilerListener.h"
#include <OgreMaterialManager.h>
#include <OgreLogManager.h>
#include <OgreResourceGroupManager.h>
#include <sstream>

namespace HovUni {

DuplicateMaterialScriptCompilerListener::DuplicateMaterialScriptCompilerListener(Ogre::ScriptCompilerListener* chainedListener)
	: mChainedListener(chainedListener), mNextScratchId(0) {
}

DuplicateMaterialScriptCompilerListener::~DuplicateMaterialScriptCompilerListener() {
}

bool DuplicateMaterialScriptCompilerListener::handleEvent(Ogre::ScriptCompiler* compiler, Ogre::ScriptCompilerEvent* evt, void* retval) {
	if (evt->mType == Ogre::CreateMaterialScriptCompilerEvent::eventType) {
		Ogre::CreateMaterialScriptCompilerEvent* materialEvt = static_cast<Ogre::CreateMaterialScriptCompilerEvent*>(evt);

		// Resource names are effectively global -- look for an existing
		// material by name across *all* resource groups, not just the one
		// this script is currently being parsed into.
		Ogre::MaterialPtr existing = Ogre::MaterialManager::getSingleton().getByName(
			materialEvt->mName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

		if (existing) {
			if (mAlreadyLoggedNames.insert(materialEvt->mName).second) {
				Ogre::LogManager::getSingleton().logMessage(
					"[DuplicateMaterialScriptCompilerListener] Restoring Ogre 1.7 behaviour: material '"
					+ materialEvt->mName + "' is redefined in '" + materialEvt->mFile
					+ "' -- keeping the first definition and discarding this redefinition.");
			}

			// Do NOT hand back 'existing': MaterialTranslator unconditionally
			// parses the duplicate block's contents into whatever Material*
			// we return (it even calls removeAllTechniques() on it first),
			// so that would make the duplicate block overwrite the original
			// -- last-definition-wins, the opposite of Ogre 1.7's behaviour.
			// Instead hand back a uniquely-named scratch material for the
			// duplicate block to harmlessly clobber; the real, first-defined
			// material is left completely untouched and nothing else ever
			// refers to the scratch material by name.
			std::ostringstream scratchName;
			scratchName << "__DuplicateMaterialScriptCompilerListener/discarded/"
				<< materialEvt->mName << "/" << (mNextScratchId++);

			Ogre::MaterialPtr scratch = Ogre::MaterialManager::getSingleton().create(
				scratchName.str(), materialEvt->mResourceGroup);

			*static_cast<Ogre::Material**>(retval) = scratch.get();
			return true;
		}

		// No material by this name exists yet -- fall through to the
		// chained listener (if any) / Ogre's normal creation path below.
	}

	if (mChainedListener) {
		return mChainedListener->handleEvent(compiler, evt, retval);
	}

	return false;
}

void DuplicateMaterialScriptCompilerListener::install() {
	Ogre::ScriptCompilerListener* existing = Ogre::ScriptCompilerManager::getSingleton().getListener();
	Ogre::ScriptCompilerManager::getSingleton().setListener(new DuplicateMaterialScriptCompilerListener(existing));
}

}
