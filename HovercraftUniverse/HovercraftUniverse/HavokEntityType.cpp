#include "HavokEntityType.h"

#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Physics/Dynamics/World/hkpWorldObject.h>

namespace HovUni {

//the ID for our property randomly chosen
int HavokEntityType::ENITYTYPEPROPERTY = 15454;

HavokEntityType::Type HavokEntityType::getEntityType( const hkpWorldObject * object ){
	//mostly read only field, ignore the MT check
	if ( object->hasProperty(ENITYTYPEPROPERTY) ){
		hkpPropertyValue p = object->getProperty(ENITYTYPEPROPERTY);		
		return (HavokEntityType::Type) p.getInt();
	} else {
		return NOTSET;
	}
}

bool HavokEntityType::isEntityType( const hkpWorldObject * object, Type type ) {
	if ( object->hasProperty(ENITYTYPEPROPERTY)){
		hkpPropertyValue p = object->getProperty(ENITYTYPEPROPERTY);		
		return (HavokEntityType::Type) p.getInt() == type;
	}
	else {
		return false;
	}
}

void HavokEntityType::updateEntityType( hkpWorldObject * object, HavokEntityType::Type type ) {
	hkpPropertyValue p;
	p.setInt((int)type);
	object->editProperty(ENITYTYPEPROPERTY,p);
}

void HavokEntityType::setEntityType( hkpWorldObject * object, HavokEntityType::Type type ) {
	object->unlockPropertiesFromLoadedObject ();
	hkpPropertyValue p;
	p.setInt((int)type);
	object->addProperty(ENITYTYPEPROPERTY,p);	
}

}