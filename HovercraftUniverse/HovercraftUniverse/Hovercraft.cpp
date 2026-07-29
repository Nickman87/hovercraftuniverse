#include "Hovercraft.h"
#include <OgreStringConverter.h>

#include "String_Replicator.h"
#include "RacePlayer.h"
#include "PlayerSettings.h"
#include "RaceState.h"
#include "VisualEventParser.h"
#include "VisualEvent.h"

namespace HovUni {

const Ogre::String Hovercraft::CATEGORY("Hovercraft");

const Ogre::Real Hovercraft::MAXSPEED(200.0);

Hovercraft::Hovercraft(const Ogre::String& name, unsigned int playerId, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval):
HovercraftUniverseEntity(name,CATEGORY,position,orientation,ogreentity,processInterval,9), mPlayer(0), mTilt(0.0f), mCollisionState(false), mFinished(false), mPlayerId(playerId), mCollisionEvent(0) {
}

Hovercraft::Hovercraft( ZCom_BitStream* announcedata ): HovercraftUniverseEntity(announcedata,CATEGORY,9), mPlayer(0), mCollisionEvent(0) {
	// Display name
	int length = announcedata->getInt(sizeof(int) * 8);
	if (length != 0) {
		mDisplayName = Ogre::String(announcedata->getString());
	} else {
		mDisplayName = "";
	}
	Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "Hovercraft :: onCLientSIde: |" << mDisplayName << "|";
	mLabel = mOgreEntity;

	mPlayerId = announcedata->getInt(sizeof(unsigned int) * 8);
}

void Hovercraft::load(TiXmlElement * data) throw(ParseException){
	TiXmlNode * node;
	
	//We are loading a Hovercraft!
	if(strcmp(data->Value(),"Hovercraft") != 0){
		THROW(ParseException, "Given XML element is not of type hovercraft.");
	}

	//Read name
	mDisplayName = "No name";
	node = data->FirstChild("Name");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mDisplayName = Ogre::String(element->GetText());
		}
	}

	//Read Description
	mDescription = "";
	node = data->FirstChild("Description");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mDescription = Ogre::String(element->GetText());
		}
	}

	//Read Speed
	mMaximumSpeed = 0.0f;
	node = data->FirstChild("Speed");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mMaximumSpeed = Ogre::StringConverter::parseReal(Ogre::String(element->GetText()));
		}
	}

	//Read mass
	mMass = 0.0f;
	node = data->FirstChild("Mass");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mMass = Ogre::StringConverter::parseReal(Ogre::String(element->GetText()));
		}
	}

	//Read Acceleration
	mAcceleration = 0.0f;
	node = data->FirstChild("Acceleration");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mAcceleration = Ogre::StringConverter::parseReal(Ogre::String(element->GetText()));
		}
	}

	//Read Steering
	mSteering = 0.0f;
	node = data->FirstChild("Steering");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mSteering = Ogre::StringConverter::parseReal(Ogre::String(element->GetText()));
		}
	}

	//set up fields that are not in xml
	mSpeed = 0.0f;
}

Hovercraft::~Hovercraft(void){
}

void Hovercraft::updateLabel() {
	if (mRacestate) {
		if (mRacestate->getOwnPlayer() && mRacestate->getOwnPlayer()->getSettings()->getID() == mPlayerId) {
			mLabel = ""; // This is you
		} else {
			mLabel = mPlayer->getSettings()->getPlayerName();
		}
	} else {
		mLabel = mOgreEntity;
	}
}

void Hovercraft::process(float timeSince){
}

void Hovercraft::setRaceState(RaceState* racestate) {
	HovercraftUniverseEntity::setRaceState(racestate);
	if (mRacestate) {
		mPlayer = mRacestate->getPlayer(mPlayerId);
	}
}

void Hovercraft::parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream, float timeSince){
	Entity::parseEvents(type,remote_role,conn_id,stream,timeSince);

	if ( type == eZCom_EventUser ){	
		VisualEventParser p;
		VisualEvent * event = p.parse(stream);
		if (event ){
			processVisualEvent(event);
			delete event;
		}
	}
}


void Hovercraft::processEventsOther(ControllerEvent* cEvent){
}

void Hovercraft::processVisualEvent (VisualEvent * e){
	switch (e->getType()){
		case onCollision:
			{
				CollisionEvent * colevent = dynamic_cast<CollisionEvent*>(e);
				delete mCollisionEvent;
				mCollisionEvent = new CollisionEvent(*colevent);
				//const Ogre::Vector3& normal = colevent->getNormal();
				//const Ogre::Vector3& position = colevent->getPosition();
				//std::cout << "GOT COLLISION EVENT " << position[0] << " " << position[1] << " " << position[2] << std::endl;
			}
			break;
		default:
			break;
	};
}

CollisionEvent* Hovercraft::getCollisionEvent() const {
	return mCollisionEvent;
}

void Hovercraft::processEventsServer(ControllerEvent* cEvent){
	// Save the new event in the moving status
	BasicEntityEvent* movestatus = dynamic_cast<BasicEntityEvent*>(cEvent);
	if (mControlsActive && !mFinished) {
		if (movestatus) {
		mMovingStatus = *movestatus;
		}
	} else {
		mMovingStatus.clear();
	}
}

void Hovercraft::setupReplication(){
	//set up replication
	Entity::setupReplication();

	//name
	replicateString(&mDisplayName, ZCOM_REPRULE_AUTH_2_ALL);

	//mDescription
	mNode->addReplicator(
		new String_Replicator(&mDescription,
		ZCOM_REPFLAG_MOSTRECENT,
		ZCOM_REPRULE_AUTH_2_ALL
	), 
	true);

	//mAcceleration
	mNode->addReplicationFloat(&mAcceleration,
	4,
	ZCOM_REPFLAG_MOSTRECENT,
	ZCOM_REPRULE_AUTH_2_ALL
	);

	//mMass
	mNode->addReplicationFloat(&mMass,
	4,
	ZCOM_REPFLAG_MOSTRECENT,
	ZCOM_REPRULE_AUTH_2_ALL
	);

	//mMaximumSpeed
	mNode->addReplicationFloat(&mMaximumSpeed,
	4,
	ZCOM_REPFLAG_MOSTRECENT,
	ZCOM_REPRULE_AUTH_2_ALL
	);

	//mSpeed
	mNode->addReplicationFloat(&mSpeed,
	4,
	ZCOM_REPFLAG_MOSTRECENT,
	ZCOM_REPRULE_AUTH_2_ALL
	);
	//mSteering
	mNode->addReplicationFloat(&mSteering,
	4,
	ZCOM_REPFLAG_MOSTRECENT,
	ZCOM_REPRULE_AUTH_2_ALL
	);

	//Collision state
	mNode->addReplicationBool(&mCollisionState, ZCOM_REPFLAG_MOSTRECENT, ZCOM_REPRULE_AUTH_2_ALL);

	//player id
	mNode->addReplicationInt((int*)&mPlayerId,
	sizeof(unsigned int)*8,
	false,
	ZCOM_REPFLAG_MOSTRECENT,
	ZCOM_REPRULE_AUTH_2_ALL
	);
}

void Hovercraft::setAnnouncementData(ZCom_BitStream* stream) {
	Entity::setAnnouncementData(stream);
	
	stream->addInt(mDisplayName.length(),sizeof(int) * 8);
	if ( mDisplayName.length() != 0 ){
		stream->addString(mDisplayName.c_str());
	}

	stream->addInt(mPlayerId,sizeof(unsigned int) * 8);
}

std::string Hovercraft::getClassName(){
	return "Hovercraft";
}

}
