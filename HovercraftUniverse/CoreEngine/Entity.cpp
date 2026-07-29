#include "Entity.h"
#include "EntityManager.h"
#include "ControllerEventParser.h"
#include <cassert>
#include <OgreLogManager.h>
#include "EntityPropertySystem.h"

namespace HovUni {

class EntityManager;

Entity::Entity(const Ogre::String& name, const Ogre::String& category, const Ogre::Vector3& position, const Ogre::Vector3& orientation, const Ogre::Vector3& upvector, const Ogre::String& ogreentity, float processInterval, unsigned short replicators) : 
		NetworkEntity(replicators + 6), mName(name), mCategory(category), mOgreEntity(ogreentity), mController(0), mProcessInterval(processInterval),
			mProcessElapsed(processInterval), mPosition(Ogre::Vector3::ZERO), mVelocity(Ogre::Vector3::ZERO), mOrientation(Ogre::Quaternion::IDENTITY), mLabel(""), mTmpPosition(Ogre::Vector3::ZERO) {
	// Update data
	changePosition(position);
	changeOrientation(Ogre::Vector3::UNIT_Y.getRotationTo(upvector));
	changeOrientation(Ogre::Vector3::NEGATIVE_UNIT_Z.getRotationTo(orientation));

	init();
}

Entity::Entity(const Ogre::String& name, const Ogre::String& category, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval, unsigned short replicators) : 
		NetworkEntity(replicators + 6), mName(name), mCategory(category), mOgreEntity(ogreentity), mController(0), mProcessInterval(processInterval),
		mProcessElapsed(processInterval), mPosition(Ogre::Vector3::ZERO), mVelocity(Ogre::Vector3::ZERO), mOrientation(Ogre::Quaternion::IDENTITY), mLabel(""), mTmpPosition(Ogre::Vector3::ZERO) {
	// Update data
	changePosition(position);
	changeOrientation(orientation);

	init();
}

Entity::Entity ( ZCom_BitStream* announcementdata, const Ogre::String& category, unsigned short replicators ): 
	NetworkEntity(replicators + 6), mController(0), mCategory(category), mPosition(Ogre::Vector3::ZERO), mVelocity(Ogre::Vector3::ZERO), mLabel(""), mTmpPosition(Ogre::Vector3::ZERO)
{
	//name and entity

	int length;
	
	length = announcementdata->getInt(sizeof(int) * 8);
	if ( length != 0 ){
		mName = Ogre::String(announcementdata->getString());

		/*char * buffer = new char[length + 1];
		announcementdata->getString(buffer,length+1);
		buffer[length] = '\0';
		mName = Ogre::String(buffer);
		delete [] buffer;*/
	}
	else {
		mName = "";
	}

	length = announcementdata->getInt(sizeof(int) * 8);
	if ( length != 0 ){
		mOgreEntity = Ogre::String(announcementdata->getString());

		/*char * buffer = new char[length + 1];
		announcementdata->getString(buffer,length);
		buffer[length] = '\0';
		mOgreEntity = Ogre::String(buffer);
		delete [] buffer;*/
	}
	else {
		mOgreEntity = "";
	}

	//process info
	mProcessInterval = announcementdata->getFloat(10);
	mProcessElapsed = mProcessInterval;

	//position
	mPosition[0] = announcementdata->getFloat(10);
	mPosition[1] = announcementdata->getFloat(10);
	mPosition[2] = announcementdata->getFloat(10);
	mTmpPosition = mPosition;
	
	//orientation
	mOrientation[0] = announcementdata->getFloat(10);
	mOrientation[1] = announcementdata->getFloat(10);
	mOrientation[2] = announcementdata->getFloat(10);
	mOrientation[3] = announcementdata->getFloat(10);

	init();
}


Entity::~Entity() {
	delete mController;
	// Empty
	delete mProperties;
}

void Entity::init() {
	mSpringInterpolator.setDampening(60);
	mSpringInterpolator.setMass(60);
	mSpringInterpolator.setStiffness(30);

	mProperties = new EntityPropertyMap(this);

	mLastPosition = Ogre::Vector3::ZERO;
}

void Entity::changePosition(const Ogre::Vector3& newPosition) {
	// TODO Check whether valid

	if (mPosition == Ogre::Vector3::ZERO) {
		mTmpPosition = newPosition;
	}
	// Set new position
	mPosition = newPosition;
	mLastPosition = newPosition;
}

void Entity::changeVelocity(const Ogre::Vector3& newVelocity) {
	mVelocity = newVelocity;
}

void Entity::changeOrientation(const Ogre::Quaternion& newOrientation) {
	// TODO Check whether valid

	// Set new orientation
	mOrientation = newOrientation;
}

void Entity::setController(Controller * controller) {
	// TODO Check whether valid

	// Set controller
	mController = controller;
	mController->setEntity(this);
}

void Entity::update(float timeSince) {
	/*
	if (this->getCategory() == "Hovercraft") {
		std::cout << "Position: " << mPosition << std::endl;
		std::cout << "tmpPosition: " << mTmpPosition << std::endl;
	}
	*/

	//if (mNode->getRole() != eZCom_RoleAuthority) {
		//Interpolate the position manually
		//mTmpPosition = mSpringInterpolator.updateCameraSpring(mTmpPosition, mPosition);
	//}

	if (mLastPosition != mPosition) {
		mTmpPosition = mPosition;
		mLastPosition = mPosition;
	} else {
		//Predict a new position
		mTmpPosition += mVelocity * timeSince;
	}
	
	// Process the network entity
	NetworkEntity::processEvents(timeSince);

	// Process controller
	processController(timeSince);

	if (mProcessInterval != -1) {
		mProcessElapsed -= timeSince;
		// Fire callback to process entity specific
		float since = mProcessInterval - mProcessElapsed;
		while (mProcessElapsed <= 0.0f) {
			process(since);
			mProcessElapsed += mProcessInterval;
			since = 0.0f;
		}
	}
}

Ogre::String Entity::getName() const { 
	return mName;
}

Ogre::String Entity::getLabel() const { 
	return mLabel;
}

bool Entity::hasLabel() const {
	return (getLabel() != "");
}

Ogre::String Entity::getCategory() const { 
	return mCategory;
}

Ogre::Vector3 Entity::getPosition() const {
	//return mPosition;
	return mTmpPosition;
}

Ogre::Vector3 Entity::getSmoothPosition() const {
	return mTmpPosition; 
}

Ogre::Vector3 Entity::getVelocity() const { 
	return mVelocity; 
}

Ogre::Vector3 Entity::getUpVector() const { 
	return mOrientation * Ogre::Vector3::UNIT_Y; 
}

Ogre::Vector3 Entity::getOrientation() const {
	return mOrientation * Ogre::Vector3::NEGATIVE_UNIT_Z;
}

Ogre::String Entity::getOgreEntity() const {
	return mOgreEntity;
}

Ogre::Quaternion Entity::getQuaternion() const {
	return mOrientation;
}

EntityPropertyMap * Entity::getPropertyMap() {
	return mProperties;
}

void Entity::parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream, float timeSince) {
	//If user event
	if ( type == eZCom_EventUser ){	
		//Control events
		{
			ControllerEventParser p;
			ControllerEvent* event = p.parse(stream);
			if ( event ){
				processControllerEvents(event);
				delete event;
			}
		}
	}
}

void Entity::processController(float timeSince) {
	if (mController) {
		// Get events from controller
		std::vector<ControllerEvent*> events = mController->getEvents();
	
		// Send the events
		for (std::vector<ControllerEvent*>::iterator it = events.begin(); it != events.end(); ++it) {
			ControllerEvent* event = *it;
			if (isRegistered() && mNode->getRole() != eZCom_RoleAuthority) {
				sendEvent(*event);
			} else {
				processControllerEvents(event);
			}
			delete event;
		}
	}
}

void Entity::processControllerEvents(ControllerEvent* event) {
	eZCom_NodeRole role = mNode->getRole();
	switch (role) {
		case eZCom_RoleAuthority:
			//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "Event for server";
			processEventsServer(event);
			//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "Processed event for server";
			break;
		case eZCom_RoleOwner:
			//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "Event for owner"; 
			processEventsOwner(event);
			break;
		case eZCom_RoleProxy:
			processEventsOther(event);
			break;
		default:
			break;
	}
}

void Entity::setupReplication() {
	//TODO: min and max delay should be defined, not magic number...
	replicateOgreVector3(&mPosition, ZCOM_REPRULE_AUTH_2_ALL, 23, ZCOM_REPFLAG_MOSTRECENT, 0, -1, 1000);
	//mNode->addInterpolationFloat(&mPosition.x, 10, ZCOM_REPFLAG_MOSTRECENT, ZCOM_REPRULE_AUTH_2_ALL, 0, &mTmpPosition.x);
	//mNode->addInterpolationFloat(&mPosition.y, 10, ZCOM_REPFLAG_MOSTRECENT, ZCOM_REPRULE_AUTH_2_ALL, 0, &mTmpPosition.y);
	//mNode->addInterpolationFloat(&mPosition.z, 10, ZCOM_REPFLAG_MOSTRECENT, ZCOM_REPRULE_AUTH_2_ALL, 0, &mTmpPosition.z);
	//replicateInterpolationFloat(&mPosition.x, 10.0f, ZCOM_REPRULE_AUTH_2_ALL, 23, ZCOM_REPFLAG_MOSTRECENT, 100, 1000, 0, &mTmpPosition.x); 
	//replicateInterpolationFloat(&mPosition.y, 10.0f, ZCOM_REPRULE_AUTH_2_ALL, 23, ZCOM_REPFLAG_MOSTRECENT, 100, 1000, 0, &mTmpPosition.y);
	//replicateInterpolationFloat(&mPosition.z, 10.0f, ZCOM_REPRULE_AUTH_2_ALL, 23, ZCOM_REPFLAG_MOSTRECENT, 100, 1000, 0, &mTmpPosition.z);

	replicateOgreQuaternion(&mOrientation, ZCOM_REPRULE_AUTH_2_ALL, 23, ZCOM_REPFLAG_MOSTRECENT, 0, -1, 1000);
	replicateOgreVector3(&mVelocity, ZCOM_REPRULE_AUTH_2_ALL, 23, ZCOM_REPFLAG_MOSTRECENT, 0, -1, 1000);

	mNode->addReplicator(new EntityPropertyMapReplicator(*mProperties,ZCOM_REPRULE_AUTH_2_ALL,ZCOM_REPFLAG_MOSTRECENT),true);
}

void Entity::setAnnouncementData(ZCom_BitStream* stream) {
	
	stream->addInt(mName.length(),sizeof(int) * 8);
	if ( mName.length() != 0 ){
		stream->addString(mName.c_str());
	}

	stream->addInt(mOgreEntity.length(),sizeof(int) * 8);
	if ( mOgreEntity.length() != 0 ){
		stream->addString(mOgreEntity.c_str());
	}

	stream->addFloat(mProcessInterval,10);
	
	stream->addFloat(mPosition[0],10);
	stream->addFloat(mPosition[1],10);
	stream->addFloat(mPosition[2],10);

	stream->addFloat(mOrientation[0],10);
	stream->addFloat(mOrientation[1],10);
	stream->addFloat(mOrientation[2],10);
	stream->addFloat(mOrientation[3],10);
}

void Entity::setControlsActive() {
	mControlsActive = true;
}

void Entity::setControlsInactive() {
	mControlsActive = false;
}

void Entity::toggleControlsActive() {
	mControlsActive = !mControlsActive;
}

bool Entity::mControlsActive = false;

}
