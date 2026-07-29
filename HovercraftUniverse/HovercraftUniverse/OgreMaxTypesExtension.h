#ifndef OGREMAXTYPESEXTENSION_H
#define OGREMAXTYPESEXTENSION_H

#include <OgreMaxTypes.hpp>

namespace OgreMax {

namespace Types {

enum AttachableType {
	NODE,
	ENTITY
};

/**
 * Container for an attachables
 */
struct Attachable {
	AttachableType type;

	Ogre::String name;

	virtual ~Attachable(){};
};

/**
 * SceneNode: (Ogre::SceneNode)
 */
struct SceneNode : Attachable {
	SceneNode(){
		type = NODE;
	}
};

/**
 * Tagpoint (Ogre::TagePoint)
 * attached to an entity (ex. point at hand for holding gun)
 */
struct TagPoint : Attachable {
	TagPoint(){
		type = ENTITY;
	}

	Ogre::String tagName;
	Ogre::String boneName;
	Ogre::Vector3 attachPosition;
	Ogre::Quaternion attachRotation;
	Ogre::Vector3 attachScale;
};



/**
 * A Keyframe
 */
struct KeyFrame {
	Ogre::Real keyTime;
	Ogre::Real keyframe;
	Ogre::Vector3 translation;
	Ogre::Quaternion rotation;
	Ogre::Vector3 scale;
};

struct NodeAnimation {
	NodeAnimationParameters parameters;
	std::vector<KeyFrame> animationTrack;
};


/**
 * A Billboard
 */
struct Billboard {
    Ogre::Real width;
    Ogre::Real height;
    Ogre::Radian rotationAngle;

    Ogre::Vector3 position;
    Ogre::ColourValue color;
    Ogre::FloatRect texCoordRectangle;
};

}
}

#endif