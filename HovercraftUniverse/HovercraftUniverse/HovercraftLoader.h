#ifndef HOVERCRAFTLOADER_H
#define HOVERCRAFTLOADER_H

#include "Loader.h"
#include <OgreString.h>

namespace HovUni {

/**
 * Loads hovercrafts.
 * 
 * @author Kristof Overdulve
 */
class HovercraftLoader : public Loader {
private:

	/** The scene manager */
	Ogre::SceneManager * mSceneMgr;

	/** The current entity */
	EntityRepresentation * mCurrEntity;

	/** The current node */
	Ogre::SceneNode * mCurrNode;

	/** The name of the hovercraft to load */
	Ogre::String mHovName;

public:
	HovercraftLoader(Ogre::SceneManager * sceneMgr, Ogre::String hovName);

	virtual ~HovercraftLoader(void);

	virtual void StartedLoad();

	virtual void FinishedLoad( bool success );

	virtual void onEntity( OgreMax::Types::EntityParameters& entityparameters, const OgreMax::Types::Attachable * parent );

	virtual void onNode( OgreMax::Types::NodeParameters& nodeparameters, std::vector<OgreMax::Types::NodeAnimation> * animation, const OgreMax::Types::NodeParameters* parent);
};

}

#endif