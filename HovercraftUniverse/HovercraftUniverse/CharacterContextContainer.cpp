#include "CharacterContextContainer.h"

namespace HovUni {

	CharacterContextContainer::CharacterContextContainer()
	{
		//create default
		{
			hkpCharacterState* state;
			hkpCharacterStateManager* manager = new hkpCharacterStateManager();

			state = new hkpCharacterStateOnGround();
			manager->registerState( state,	HK_CHARACTER_ON_GROUND);
			state->removeReference();

			state = new hkpCharacterStateInAir();
			manager->registerState( state,	HK_CHARACTER_IN_AIR);
			state->removeReference();

			state = new hkpCharacterStateJumping();
			manager->registerState( state,	HK_CHARACTER_JUMPING);
			state->removeReference();

			state = new hkpCharacterStateClimbing();
			manager->registerState( state,	HK_CHARACTER_CLIMBING);
			state->removeReference();

			hkpCharacterContext * characterContext = new hkpCharacterContext(manager, HK_CHARACTER_IN_AIR);	
			manager->removeReference();			

			// Set new filter parameters for final output velocity filtering
			// Smoother interactions with small dynamic boxes
			characterContext->setCharacterType(hkpCharacterContext::HK_CHARACTER_RIGIDBODY);
			characterContext->setFilterParameters(0.9f,12.0f,200.0f);

			registerContext("Default", characterContext);

			characterContext->removeReference();
		}
	}

	CharacterContextContainer::~CharacterContextContainer(){
		CharacterContextMap::Iterator i = mMap.getIterator();
		while ( mMap.isValid(i) ){
			hkpCharacterContext * value = mMap.getValue(i);
			value->removeReference();	//remove the reference added in register
			mMap.setValue(i, HK_NULL);
			i = mMap.getNext(i);
		}
		mMap.clear();
	}

	void CharacterContextContainer::registerContext( const char * key, hkpCharacterContext * context ) {	
		context->addReference();	//add a reference since we add it to our map
		mMap.insert(key,context);
	}

	
	hkpCharacterContext * CharacterContextContainer::getCharacterContext( const char * key ){
		hkpCharacterContext * context;
		if ( mMap.get(key,&context) == HK_SUCCESS ){
			context->addReference();
			return context;
		}
		return HK_NULL;
	
	}

}
