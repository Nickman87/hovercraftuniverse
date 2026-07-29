#include "EntityPropertySystem.h"
#include "Entity.h"

#include <list>

namespace HovUni {

//// MAP FACT ///

boost::mutex EntityPropertyFactory::mMutex;
std::map<int,EntityProperty*(*)()> * EntityPropertyFactory::mFactory = 0;

void EntityPropertyFactory::add(int id,EntityProperty*(*r)()){
	boost::mutex::scoped_lock lock(mMutex);

	if (!mFactory){
		mFactory = new std::map<int,EntityProperty*(*)()>();
	}
	
	mFactory->insert(std::pair<int,EntityProperty*(*)()>(id,r));
}

void EntityPropertyFactory::remove(int id){
	boost::mutex::scoped_lock lock(mMutex);

	if (mFactory){
		mFactory->erase(id);

		if (mFactory->empty()){
			delete mFactory;
			mFactory = 0;
		}
	}
}

EntityProperty* EntityPropertyFactory::create(int id){
	boost::mutex::scoped_lock lock(mMutex);

	if (mFactory){
		std::map<int,EntityProperty*(*)()>::iterator i = mFactory->find(id);
		if ( i == mFactory->end() )
			return 0;
		else
			return i->second();
	}
	else {
		return 0;
	}
}


//// MAP ///

EntityPropertyMap::EntityPropertyMap(Entity * entity):
	mEntity(entity), mUpdate(0) {
}

EntityPropertyMap::~EntityPropertyMap(){
	
	//destroy all properties
	for (PropertyMap::iterator i = mMap.begin(); i != mMap.end(); i++){
		delete i->second;
		i->second = 0;
	}
	mMap.clear();
}

bool EntityPropertyMap::addProperty(EntityProperty * property ){	
	boost::mutex::scoped_lock lock(mMutex);

	property->mEntity = mEntity;
	
	std::pair<PropertyMap::iterator,bool> r = mMap.insert(Property(property->getKey(),property));
	
	update();

	return r.second;
}

bool EntityPropertyMap::hasProperty(int key) const {
	return mMap.find(key) != mMap.end();
}

EntityProperty * EntityPropertyMap::getProperty(int key){
	boost::mutex::scoped_lock lock(mMutex);

	PropertyMap::iterator i = mMap.find(key);
	if (i == mMap.end()){
		return 0;
	}	
	else {
		return i->second;
	}
}

bool EntityPropertyMap::removeProperty (int key, bool del){
	boost::mutex::scoped_lock lock(mMutex);

	PropertyMap::iterator i = mMap.find(key);
	if (i == mMap.end()){
		return false;
	}	
	else {
		if (del){
			delete i->second;
			i->second = 0;
		}

		mMap.erase(i);

		update();

		return true;
	}
}

bool EntityPropertyMap::removeProperty (EntityProperty * property, bool del){
	return removeProperty(property->getKey(),del);
}

EntityProperty * EntityPropertyMap::removeProperty (int key){
	boost::mutex::scoped_lock lock(mMutex);

	PropertyMap::iterator i = mMap.find(key);
	if (i == mMap.end()){
		return 0;
	}	
	else {
		EntityProperty * result = i->second;
		mMap.erase(i);
	
		update();

		return result;
	}
}

EntityProperty * EntityPropertyMap::removeProperty (EntityProperty * property){
	boost::mutex::scoped_lock lock(mMutex);

	PropertyMap::iterator i = mMap.find(property->getKey());
	if (i == mMap.end()){
		return 0;
	}	
	else {
		mMap.erase(i);

		update();

		return property;
	}
}

void EntityPropertyMap::update(){
	mUpdate++;
}

// PROPERTY //

EntityProperty::EntityProperty( int id ):
	mEntity(0), mVersion(0), mKey(id){
}

EntityProperty::~EntityProperty(void){
}

Entity * EntityProperty::getEntity(){
	return mEntity;
}

void EntityProperty::update(){
	mVersion++;
}

int EntityProperty::getKey() const{
	return mKey;
}

signed char EntityProperty::getVersion() const{
	return mVersion;
}

// REPLICATOR //

EntityPropertyMapReplicator::EntityPropertyMapReplicator (EntityPropertyMap& data, ZCom_ReplicatorSetup * setup):
	ZCom_ReplicatorAdvanced(setup), mData(data), mTimeout(0), mUpdate(data.mUpdate) {

	for (PropertyMap::iterator i = mData.mMap.begin(); i != mData.mMap.end(); i++){
		mCopy.insert(std::pair<int,int>(i->first,i->second->getVersion()));
	}

	m_flags |= ZCOM_REPLICATOR_INITIALIZED;
	m_flags |= ZCOM_REPLICATOR_CALLPROCESS;
}

EntityPropertyMapReplicator::EntityPropertyMapReplicator(EntityPropertyMap& data, zU8 _flags, zU8 _rules, zU8 _intercept_id, zS16 _mindelay, zS16 _maxdelay):
	ZCom_ReplicatorAdvanced(NULL), mData(data), mTimeout(0), mUpdate(data.mUpdate) {
	
	for (PropertyMap::iterator i = mData.mMap.begin(); i != mData.mMap.end(); i++){
		mCopy.insert(std::pair<int,int>(i->first,i->second->getVersion()));
	}
		
	// make sure m_setup will be deleted when the replicator is deleted
	_flags |= ZCOM_REPFLAG_SETUPAUTODELETE;

	// create the setup object
	m_setup = new ZCom_ReplicatorSetup(_flags, _rules, _intercept_id, _mindelay, _maxdelay);

	// out of mem
	if (!m_setup) {
		return;
	}

	m_flags |= ZCOM_REPLICATOR_INITIALIZED;
	m_flags |= ZCOM_REPLICATOR_CALLPROCESS;
}

void EntityPropertyMapReplicator::updateMap(ZCom_BitStream &_stream){
	//parse the info and update the map	

	//get the number of records (ADD, UPDATE , REMOVE)
	zU8 records = _stream.getInt(sizeof(zU8)*8);

	for (zU8 i = 0; i < records; i++){

		//Get the type of the record
		zU8 type = _stream.getInt(sizeof(zU8)*2);	//2BITS

		switch (type){
			case UPDATE:	
				{
					//Get the property the action works on
					int proptype = _stream.getSignedInt(sizeof(int)*8);

					//read new fields
					EntityProperty * property = mData.getProperty(proptype);
					assert(property); //THIS MUST EXISTS ON THE CLIENT TOO!!
					property->read(_stream);
					std::cout << "UPDATE " << property->getKey() << std::endl;
					break;
				}
			case ADD:
				{		
					//Get the property the action works on
					int proptype = _stream.getSignedInt(sizeof(int)*8);

					//lookup AND add				
					EntityProperty * property = EntityPropertyFactory::create(proptype);
					assert(property);	//THIS MUST EXISTS ON THE CLIENT TOO!!
					property->read(_stream);
					mData.addProperty(property);
					std::cout << "ADD " << property->getKey() << std::endl;
					break;
				}
			case REMOVE:
				{
					//Get the property the action works on
					int proptype = _stream.getSignedInt(sizeof(int)*8);

					//kill it
					bool success = mData.removeProperty(proptype,true);
					std::cout << "REMOVED " << proptype << std::endl;
					//assert(success);
					break;
				}
			case RESET:
				{
					PropertyMap newmap;

					int mapentries = _stream.getInt(sizeof(int)*8);
					for ( int i = 0; i < mapentries; i++ ){
						int propkey = _stream.getSignedInt(sizeof(int)*8);
						EntityProperty * property = EntityPropertyFactory::create(propkey);
						property->read(_stream);	
						newmap.insert(Property(propkey,property));
					}

					//overwrite the map
					mData.mMutex.lock();
					mData.mMap = newmap;	
					mData.mMutex.unlock();

					//std::cout << "RESET" << std::endl;

					break;
				}
		};
	}
}

void EntityPropertyMapReplicator::storeAdd(EntityProperty * property, ZCom_BitStream &stream){
	//write action	
	stream.addInt(ADD,sizeof(zU8)*2);
	//write the type
	stream.addSignedInt(property->getKey(),sizeof(int)*8);
	//write the object
	property->write(stream);		
}

void EntityPropertyMapReplicator::storeRemove(int propertykey, ZCom_BitStream &stream){
	//write action	
	stream.addInt(REMOVE,sizeof(zU8)*2);
	//write the type
	stream.addSignedInt(propertykey,sizeof(int)*8);
}

void EntityPropertyMapReplicator::storeUpdate(EntityProperty * property, ZCom_BitStream &stream){
	//write action	
	stream.addInt(UPDATE,sizeof(zU8)*2);
	//write the type
	stream.addSignedInt(property->getKey(),sizeof(int)*8);
	//write the object
	property->write(stream);
}

void EntityPropertyMapReplicator::onConnectionAdded (ZCom_ConnID _cid, eZCom_NodeRole _remoterole){

	/*//if i'm the server send the entire map info to the new guy
	if (getNode()->getRole() == eZCom_RoleAuthority){

		//for every entry in my property map send an ADD command to the new client
		ZCom_BitStream * stream = new ZCom_BitStream();

		//version 
		stream->addSignedInt(mData.mUpdate,sizeof(zS8)*8);
		
		//number of records
		stream->addInt(mData.mMap.size(),sizeof(zU8)*8);

		//an add command for all
		for (PropertyMap::iterator i = mCopy.begin(); i != mCopy.end(); i++){
			storeAdd(i->second,*stream);
		}

		sendDataDirect(eZCom_ReliableUnordered,_cid,stream);
	}*/

	//TODO 
}

void EntityPropertyMapReplicator::onDataReceived (ZCom_ConnID _cid, eZCom_NodeRole _remoterole, ZCom_BitStream &_stream, bool _store, zU32 _estimated_time_sent){
	//Data from server is cool
	if (_remoterole == eZCom_RoleAuthority){

		//version
		zS8 version = _stream.getSignedInt(sizeof(zS8)*8);

		//std::cout << "RECEIVED version=" << (int)version << std::endl;

		updateMap(_stream);
	}


	//Server that receives data from owner might be cool too
	/*else if (_remoterole == eZCom_RoleOwner && getNode()->getRole() == eZCom_RoleAuthority){

		zS8 version = _stream.getSignedInt(sizeof(zS8)*8);

		if (!_store){
			updateMap(_stream,false);
		}
		else {
			//TODO FIX SOME SYSTEM THAT DOES PERMISSIONS??
			//TODO versie check
			bool newupdate = true;

			if (newupdate){
				updateMap(_stream,true);
			}
			else{
				updateMap(_stream,false);
			}
		}
		//SYNC WITH OTHER CLIENTS (happens next update..)
	}*/
}

void EntityPropertyMapReplicator::Process (eZCom_NodeRole _localrole, zU32 _simulation_time_passed){
	
	//if server check for updates and send them to clients
	if (_localrole == eZCom_RoleAuthority ){
		//lock properties
		mData.mMutex.lock();

		mTimeout += _simulation_time_passed;
		if ( mTimeout > DEFAULT_TIMEOUT ){
			mTimeout = 0;
	
			//send entire map in case if someone missed something

			ZCom_BitStream * bs = new ZCom_BitStream();

			//add newUpdate
			bs->addSignedInt(mData.mUpdate,sizeof(zS8)*8);

			//add number of entries
			bs->addInt(1,sizeof(zU8)*8);

			//write action
			bs->addInt(RESET,sizeof(zU8)*2);

			//write map
			bs->addInt(mData.mMap.size(),sizeof(int)*8);
			for ( PropertyMap::iterator i = mData.mMap.begin(); i != mData.mMap.end(); i++){
				bs->addSignedInt(i->first,sizeof(int)*8);
				i->second->write(*bs);
			}

			//send the data
			sendData(eZCom_ReliableUnordered,bs);			
		}
		else {
			//list with updated properties
			std::list<EntityProperty*> updatedProperties;	

			//list with new properties
			std::list<EntityProperty*> newProperties;

			//list with ids that are removed
			std::list<int> removedProperties;		
		
			/*if ( !newCopy.empty() ){
				std::cout << "newCopy" << std::endl;
				for (std::map<int,int>::iterator i = newCopy.begin(); i != newCopy.end(); i++){
					std::cout << i->first << std::endl;	
				}
			}

			if ( !mCopy.empty() ){
				std::cout << "mCopy" << std::endl;
				for (std::map<int,int>::iterator i = mCopy.begin(); i != mCopy.end(); i++){
					std::cout << i->first << std::endl;	
				}	
			}*/

			//quick check if versions have changed
			if ( mUpdate != mData.mUpdate ){

				//different version means element added or removed

				//find removed elements
				for (std::map<int,int>::iterator i = mCopy.begin(); i != mCopy.end(); i++){
					if (mData.mMap.find(i->first) == mData.mMap.end()){
						removedProperties.push_back(i->first);
					}
				}

				//find new elements
				for (PropertyMap::iterator i = mData.mMap.begin(); i != mData.mMap.end(); i++){
					if (mCopy.find(i->first) == mCopy.end()){
						newProperties.push_back(i->second);
					}
				}
			}

			//find updated elements
			for (PropertyMap::iterator i = mData.mMap.begin(); i != mData.mMap.end(); i++){
				std::map<int,int>::iterator j = mCopy.find(i->first);
				if ((j != mCopy.end()) && (i->second->getVersion() != j->second)){
					updatedProperties.push_back(i->second);
				}
			}

			//changes is number of adds, removes and updates
			int changes = updatedProperties.size() + newProperties.size() + removedProperties.size();

			if (changes != 0){

				//std::cout << "UPDATE: version=" << mData.mUpdate << " changes=" << changes << std::endl;

				ZCom_BitStream * bs = new ZCom_BitStream();

				//add newUpdate
				bs->addSignedInt(mData.mUpdate,sizeof(zS8)*8);

				//add number of entries
				bs->addInt(changes,sizeof(zU8)*8);

				//Send updates
				for (std::list<EntityProperty*>::iterator i = updatedProperties.begin(); i != updatedProperties.end(); i++) {
					//send update
					//std::cout << "U: " << (*i)->getKey() << std::endl;				
					storeUpdate(*i,*bs);
				}

				//Send adds
				for (std::list<EntityProperty*>::iterator i = newProperties.begin(); i != newProperties.end(); i++) {
					//send add
					//std::cout << "A: " << (*i)->getKey() << std::endl;
					storeAdd(*i,*bs);
				}

				//Send removes
				for (std::list<int>::iterator i = removedProperties.begin(); i != removedProperties.end(); i++) {
					//send remove
					//std::cout << "R: " << *i << std::endl;
					storeRemove(*i,*bs);
				}

				//send the data
				sendData(eZCom_ReliableUnordered,bs);
			}
		}

		//Store the current status
		mCopy.clear();
		for (PropertyMap::iterator i = mData.mMap.begin(); i != mData.mMap.end(); i++){
			mCopy.insert(std::pair<int,int>(i->first,i->second->getVersion()));
		}

		mUpdate = mData.mUpdate;

		//unlock
		mData.mMutex.unlock();
	}
}



}
