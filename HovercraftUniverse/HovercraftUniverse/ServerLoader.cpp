#include "CustomOgreMaxScene.h"
#include "Exception.h"
#include "DedicatedServer.h"
#include "GameEntities.h"

//Race state
#include "RaceState.h"
#include "RacePlayer.h"
#include "PlayerSettings.h"
#include "NetworkIDManager.h"
#include "EntityManager.h"
#include "HovercraftAIController.h"

// Physics
#include "Havok.h"
#include "HoverCraftUniverseWorld.h"

namespace HovUni {

ServerLoader::ServerLoader() :
	mHovercraftWorld(0), mLoadingHovercrafts(false) {
}

ServerLoader::~ServerLoader(void) {
}

void ServerLoader::parseEntityParameters(TiXmlElement * root, Ogre::String& ogreentity, Ogre::Real& processtime){
	assert(root);

	TiXmlNode * current = root->FirstChild();
	while ( current ){
		TiXmlElement * element = dynamic_cast<TiXmlElement *>(current);			
		if ( element ) {
			if ( strcmp(element->Value(),"ProcessInterval") == 0 ){
				processtime = Ogre::StringConverter::parseReal(Ogre::String(element->GetText()));
			}
			else if ( strcmp(element->Value(),"OgreEntity") == 0 ){
				ogreentity = Ogre::String(element->GetText());
			}
		}
		current = current->NextSibling();
	}
}

void ServerLoader::parseTrackUserData(const Ogre::String& userdata){
	//check if track info
	if (userdata.empty()) {
		THROW(ParseException, "Invallid scene file: No track info found.");
	}
	
	//parse track info
	TiXmlDocument mDocument;
	mDocument.Parse(userdata.c_str());
	TiXmlElement * root = mDocument.RootElement();

	//should be XML
	if(!root){
		THROW(ParseException, "Invallid scene file: Invallid track info, no correct xml information.");
	}

	//create
	Track track;

	//load
	track.load(root);
	Ogre::String filename = ".\\levels\\" + track.getPhysicsFileName();

	mPathFilename = track.getPathFileName();

	mHovercraftWorld = Havok::getSingletonPtr();
	const char * physicsfile = filename.c_str();
	if (!mHovercraftWorld->load(physicsfile)) {
		mDocument.Clear();
		THROW(ParseException, "Could not load our physics file.");
	}

	mDocument.Clear();
}

void ServerLoader::parseWorldUserData(OgreMax::Types::ExternalItem& externalitem){
	
	if ( externalitem.userData.empty() ){
		THROW(ParseException, "Invallid scene file: No info for game entity found.");
	}

	//parse XML that describes the user data
	TiXmlDocument mDocument;
	mDocument.Parse(externalitem.userData.c_str());
	TiXmlElement * root = mDocument.RootElement();
	
	//should be XML
	if(!root){
		THROW(ParseException, "Invallid scene file: Invallid game entity info, no correct xml information.");
	}

	//Read some generic things of entities here
	Ogre::String ogreentity = "";
	Ogre::Real processtime = -1.0f;

	parseEntityParameters(root,ogreentity,processtime);

	//START
	if(strcmp(root->Value(),"Start") == 0){
		//create
		Start * entity = new Start(externalitem.name,externalitem.position, externalitem.rotation, ogreentity, processtime);
		
		//load
		entity->load(root);
		entity->setRaceState(mRaceState);

		//create Physics
		mHovercraftWorld->createStart(entity, externalitem);

		//register as entity
		EntityManager::getServerSingletonPtr()->registerEntity(entity);	

		//network register
		entity->networkRegister(NetworkIDManager::getServerSingletonPtr(), Start::getClassName(), true);
	}
	//STARTPOSITION
	else if(strcmp(root->Value(),"StartPosition") == 0){
		//create
		StartPosition * entity = new StartPosition(externalitem.name,externalitem.position, externalitem.rotation, processtime);
		
		//load
		entity->load(root);
		entity->setRaceState(mRaceState);

		//register as entity
		EntityManager::getServerSingletonPtr()->registerEntity(entity);	

		//network register
		entity->networkRegister(NetworkIDManager::getServerSingletonPtr(), StartPosition::getClassName(), true);
	}
	//FINISH
	else if(strcmp(root->Value(),"Finish") == 0){
		//create
		Finish * entity = new Finish(externalitem.name,externalitem.position, externalitem.rotation, ogreentity, processtime);
		
		//load
		entity->load(root);
		entity->setRaceState(mRaceState);

		//create Physics
		mHovercraftWorld->createFinish(entity,externalitem);

		//register as entity
		EntityManager::getServerSingletonPtr()->registerEntity(entity);	

		//network register
		entity->networkRegister(NetworkIDManager::getServerSingletonPtr(), Finish::getClassName(), true);
	}
	//CHECKPOINT
	else if(strcmp(root->Value(),"CheckPoint") == 0){
		//create
		CheckPoint * entity = new CheckPoint(externalitem.name,externalitem.position, externalitem.rotation, ogreentity, processtime);

		//load
		entity->load(root);
		entity->setRaceState(mRaceState);

		//create Physics
		mHovercraftWorld->createCheckpoint(entity,externalitem);

		//register as entity
		EntityManager::getServerSingletonPtr()->registerEntity(entity);	

		//network register
		entity->networkRegister(NetworkIDManager::getServerSingletonPtr(), CheckPoint::getClassName(), true);
	}
	//ASTEROID
	else if(strcmp(root->Value(),"Asteroid") == 0){
		//create
		Asteroid * entity = new Asteroid(externalitem.name,externalitem.position, externalitem.rotation, ogreentity, processtime);
		
		//load
		entity->load(root);
		entity->setRaceState(mRaceState);

		//create Physics
		mHovercraftWorld->createAsteroid(entity,externalitem);

		//register as entity
		EntityManager::getServerSingletonPtr()->registerEntity(entity);	
		
		//network register asteroid
		entity->networkRegister(NetworkIDManager::getServerSingletonPtr(), Asteroid::getClassName(), true);
	}
	//BOOST
	else if(strcmp(root->Value(),"Boost") == 0){
		//create
		SpeedBoost * entity = new SpeedBoost(externalitem.name,externalitem.position, externalitem.rotation, ogreentity, processtime);

		//load
		entity->load(root);
		entity->setRaceState(mRaceState);

		//physics
		mHovercraftWorld->createBoost(entity, externalitem);

		//register as entity
		EntityManager::getServerSingletonPtr()->registerEntity(entity);	

		//network register asteroid
		entity->networkRegister(NetworkIDManager::getServerSingletonPtr(), SpeedBoost::getClassName(), true);

	}
	//PORTAL
	else if(strcmp(root->Value(),"Portal") == 0){
		//create
		Portal * entity = new Portal(externalitem.name,externalitem.position, externalitem.rotation, ogreentity, processtime);
		
		//load
		entity->load(root);
		entity->setRaceState(mRaceState);

		//physics
		mHovercraftWorld->createPortal(entity,externalitem);

		//register as entity
		EntityManager::getServerSingletonPtr()->registerEntity(entity);	

		//network register asteroid
		entity->networkRegister(NetworkIDManager::getServerSingletonPtr(), Portal::getClassName(), true);
	}
	//POWERUPSPAWN
	else if(strcmp(root->Value(),"PowerupSpawn") == 0){
		//create
		PowerupSpawn * entity = new PowerupSpawn(externalitem.name,externalitem.position, externalitem.rotation, ogreentity, processtime);
		
		//load
		entity->load(root);
		entity->setRaceState(mRaceState);

		//register as entity
		EntityManager::getServerSingletonPtr()->registerEntity(entity);	

		//network register asteroid
		entity->networkRegister(NetworkIDManager::getServerSingletonPtr(), PowerupSpawn::getClassName(), true);
	}
	//RESETSPAWN
	else if(strcmp(root->Value(),"ResetSpawn") == 0){
		//create
		ResetSpawn * entity = new ResetSpawn(externalitem.name,externalitem.position, externalitem.rotation, processtime);

		//load
		entity->load(root);
		entity->setRaceState(mRaceState);

		//register as entity
		EntityManager::getServerSingletonPtr()->registerEntity(entity);	

		//network register asteroid
		//entity->networkRegister(NetworkIDManager::getServerSingletonPtr(), ResetSpawn::getClassName(), true);
	}
	//Rigid Body
	else if(strcmp(root->Value(),"StaticBody") == 0){
		//create
		StaticBody * entity = new StaticBody(externalitem.name,externalitem.position, externalitem.rotation,ogreentity, processtime);

		//load
		entity->load(root);
		entity->setRaceState(mRaceState);

		//physics
		mHovercraftWorld->createStaticBody(entity);

		//register as entity
		EntityManager::getServerSingletonPtr()->registerEntity(entity);	

		//network register asteroid
		entity->networkRegister(NetworkIDManager::getServerSingletonPtr(), StaticBody::getClassName(), true);
	}
	else {
		THROW(ParseException, "Invallid scene file: Unknown game entity found while parsing externals.\nValue is " + root->ValueStr());
		mDocument.Clear();
	}

	mDocument.Clear();
}

void ServerLoader::parseHovercraftUserData(OgreMax::Types::EntityParameters& entityparameters){
	
	//problem
	if (entityparameters.extraData->HasUserData() && entityparameters.extraData->userData.empty()) {
		THROW(ParseException, "Invallid scene file: No info for game entity found.");
	}

	//parse XML that describes the user data
	TiXmlDocument mDocument;
	mDocument.Parse(entityparameters.extraData->userData.c_str());
	TiXmlElement * root = mDocument.RootElement();

	//should be XML
	if(!root){
		THROW(ParseException, "Invallid scene file: Invallid game entity info, no correct xml information.");
	}

	//Read some generic things of entities here
	Ogre::String ogreentity = "";
	Ogre::Real processtime = -1.0f;

	parseEntityParameters(root,ogreentity,processtime);

	Ogre::String filename = ".\\hovercraft\\" + mPlayer->getSettings()->getHovercraft() + ".hkx";
	hkString hovercraftname(filename.c_str());

	//HOVERCRAFT
	if(strcmp(root->Value(),"Hovercraft") == 0){

		//make a unique name
		Ogre::String entityname = "Player_" + Ogre::StringConverter::toString(mPlayer->getSettings()->getID());

		//create
		Hovercraft * hovercraft(new Hovercraft(entityname,mPlayer->getSettings()->getID(),Ogre::Vector3::ZERO, Ogre::Quaternion::IDENTITY, ogreentity, processtime));
		//load
		hovercraft->load(root);

		//Get the start positions
		std::vector<Entity*> startpositions = EntityManager::getServerSingletonPtr()->getEntities(StartPosition::CATEGORY);
		StartPosition * myposition = dynamic_cast<StartPosition *>(startpositions.at(mPosition));
		Ogre::Vector3 ogre_position = myposition->getPosition();
		Ogre::Quaternion ogre_orientation = myposition->getQuaternion();

		hovercraft->changePosition(ogre_position);		
		
		//physics
		hkVector4 havok_position;
		havok_position.set(ogre_position[0],ogre_position[1],ogre_position[2]);
		hkQuaternion havok_orientation;
		havok_orientation.set(ogre_orientation.x, ogre_orientation.y, ogre_orientation.z, ogre_orientation.w);

		mHovercraftWorld->createHovercraft(hovercraft, hovercraftname,entityparameters.name.c_str(), havok_position, havok_orientation);
		
		//linkt to game
		EntityManager::getServerSingletonPtr()->registerEntity(hovercraft);

		//link to gamestate
		hovercraft->setRaceState(mRaceState);

		
		// TODO Is this the correct place to add the AI controller?
		if (mPlayer->isBot()) {
			HovercraftAIController* ai = new HovercraftAIController(DedicatedServer::getConfig()->getValue<std::string>("Server", "BotAI", "scripts/AI/PathFollowing.lua"), mPathFilename);
			hovercraft->setController(ai);
			ai->initialize();
		}

		

		//network
		hovercraft->getNetworkNode()->setOwner(mPlayer->getSettings()->getConnID(), true);
		hovercraft->networkRegister(NetworkIDManager::getServerSingletonPtr(), Hovercraft::getClassName(), true);
	}
	else {
		THROW(ParseException, "Invallid scene file: Unknown game entity found while parsing entities.\nValue is " + root->ValueStr());
		mDocument.Clear();
	}

}

void ServerLoader::load(const Ogre::String& filename) {
	Havok::start(filename.c_str(), this);
}

void ServerLoader::onSceneUserData(const Ogre::String& userDataReference, const Ogre::String& userData) {
	parseTrackUserData(userData);
}

void ServerLoader::onExternal(OgreMax::Types::ExternalItem& externalitem) {
	//ignore externals on hovercraft
	if (!mLoadingHovercrafts) {
		parseWorldUserData(externalitem);
	}
}

void ServerLoader::onEntity(OgreMax::Types::EntityParameters& entityparameters, const OgreMax::Types::Attachable * parent) {
	//should be called when loading hovercrafts
	if (mLoadingHovercrafts) {
		parseHovercraftUserData(entityparameters);
	}
}

void ServerLoader::FinishedLoad(bool success) {

	if (mLoadingHovercrafts) {
		return;
	}

	if (success) {
		//add players
		const RaceState::playermap::list_type& map = mRaceState->getPlayers();

		mLoadingHovercrafts = true;

		int j = 0;
		for (RaceState::playermap::const_iterator i = map.begin(); i != map.end(); i++) {
			//set player and position
			mPlayer = i->second;
			mPosition = j;

			//call hovercraft loader
			CustomOgreMaxScene scene;
			scene.Load(mPlayer->getSettings()->getHovercraft() + ".scene", this);

			j++;
		}

		mLoadingHovercrafts = false;
	}
}

}
