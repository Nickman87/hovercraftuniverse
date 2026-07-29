#include "HavokHovercraft.h"
#include "Havok.h"
#include "HavokEntityType.h"

#include <Common/Base/Types/Color/hkColor.h>
#include <Common/Visualize/hkDebugDisplay.h>

#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Utilities/CharacterControl/CharacterRigidBody/hkpCharacterRigidBodyListener.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Utilities/CharacterControl/CharacterRigidBody/hkpCharacterRigidBodyCinfo.h>


#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Base/Monitor/hkMonitorStream.h>
#include <Common/Base/System/Io/IStream/hkIStream.h>


#include "HoverAction.h"
#include "HavokHovercraftCollisionEffect.h"
#include "UpdatePositionAction.h"

#include "Hovercraft.h"
#include "DedicatedServer.h"
#include "EntityManager.h"
#include "CheckPoint.h"
#include "StartPosition.h"
#include "ResetSpawn.h"
#include "RacePlayer.h"

#include "BoostProperty.h"


namespace HovUni {

SimpleTest::SimpleTest( hkpWorld * world, HavokHovercraft * hover ):
	SimpleEntityCollision(world,hover, 15, hkAabb(hkVector4(-2,-2,-2),hkVector4(2,2,2))){
}

void SimpleTest::onOverlapEnter() {
	std::cout << "SimpleEntityCollision ENTER" << std::endl;
}

void SimpleTest::onOverlapLeave() {
	std::cout << "SimpleEntityCollision LEAVE" << std::endl;
}

AdvancedTest::AdvancedTest(hkpWorld * world, HavokHovercraft * hover):
	AdvancedEntityCollision(
		world,
		hover,
		2, 
		new hkpCapsuleShape(
			hkVector4(0,0,0),
			hkVector4(0,0,DedicatedServer::getEngineSettings()->getValue<float>("Collision", "ProbeLength", 10.0f)*(-1)),
			DedicatedServer::getEngineSettings()->getValue<float>("Collision", "ProbeRadius", 4.0f)
		)
	) {
}

void AdvancedTest::onOverlapEnter() {
	//std::cout << "AdvancedEntityCollision ENTER" << std::endl;
	HavokHovercraft* havokHovercraft = dynamic_cast<HavokHovercraft*>(mEntity);
	havokHovercraft->increaseCollisionCounter();
}

void AdvancedTest::onOverlapLeave() {
	//std::cout << "AdvancedEntityCollision LEAVE" << std::endl;
	HavokHovercraft* havokHovercraft = dynamic_cast<HavokHovercraft*>(mEntity);
	havokHovercraft->decreaseCollisionCounter();
}

std::ostream& operator<<(std::ostream& stream, const hkVector4& v) {
	return stream << "hkVector4(" << v.getSimdAt(0) << ", " << v.getSimdAt(1) << ", " << v.getSimdAt(2) << ", " << "w=" << v.getSimdAt(3) << ")";
}

HavokHovercraft::HavokHovercraft(hkpWorld * world, Hovercraft * entity, const hkString& filename, const hkString& entityname):
		HavokEntity(world), mEntity(entity), mFilename(filename), mEntityName(entityname), mCharacterRigidBody(HK_NULL), mCollisionCounter(0),
		mCharacterContext(HK_NULL),	mRotationDelta(DedicatedServer::getEngineSettings()->getValue<float>("Movement", "TurnAngle", 0.0f)),
		mSpeedDamping(DedicatedServer::getEngineSettings()->getValue<float>("Movement", "Damping", 0.0f)),
		mCharacterGravity(DedicatedServer::getEngineSettings()->getValue<float>("Havok", "CharacterGravity", 0.0f))
		
{
	mUp.set(0,1,0);
	mSide.set(0,0,1);
	mCollisionTest = 0;
}

HavokHovercraft::~HavokHovercraft(void)
{
	mWorld->markForWrite();

	hkpRigidBody * charbody = mCharacterRigidBody->getRigidBody();
	
	if (mCollisionTest != 0) {
		delete mCollisionTest;
	}

	//remove the body
	mWorld->removeEntity(charbody);



	mCharacterContext->removeReference();
	mCharacterContext = HK_NULL;
	
	mCharacterRigidBody->removeReference();
	mCharacterRigidBody = HK_NULL;

	mPhysicsData->removeReference();
	mPhysicsData = HK_NULL;
	
	mLoadedData->disableDestructors();
	mLoadedData->callDestructors();
	mLoadedData->removeReference();

	mWorld->unmarkForWrite();

}

const hkVector4& HavokHovercraft::getPosition() const {
	return mCharacterRigidBody->getPosition();
}

const hkVector4& HavokHovercraft::getVelocity() const {
	return mCharacterRigidBody->getLinearVelocity();
}

const hkQuaternion& HavokHovercraft::getOrientation() const {
	return mCharacterRigidBody->getRigidBody()->getRotation();
}

Entity * HavokHovercraft::getEntity() {
	return mEntity;
}

hkpRigidBody * HavokHovercraft::getRigidBody() const {
	return mCharacterRigidBody->getRigidBody();
}

void HavokHovercraft::updateUp( const hkVector4& newUp ) {
	mUp = newUp;
}

const hkVector4& HavokHovercraft::getUp() const {
	return mUp;
}

void HavokHovercraft::update(){
	//PRE STEP
	mWorld->markForWrite();

	//Update SIDE with new UP
	hkRotation rbRotation; rbRotation.set(mCharacterRigidBody->getRigidBody()->getRotation());
	hkVector4& oldForward = rbRotation.getColumn(0);
	hkVector4 newRot;
	newRot.setCross(oldForward, mUp);
	mSide.setCross(mUp, newRot);
	mSide.normalize3();
	mForward.setCross(mSide,mUp);
	
	// Update the character context
	mCharacterRigidBody->m_up = mUp;
	

	//Get the hovercraft and input
	Hovercraft * hovercraft = dynamic_cast<Hovercraft *>(mEntity);

	//##Collision state update
	//TODO (Dirk) ik heb hier de collision update gezet, = goed?
	//TODO set the '1's to '0' - but first collision detection needs a fix - the hovercraft thinks it's colliding with something it's not.
	if ((mCollisionCounter > 1) && (!hovercraft->isInCollisionState())) {
		hovercraft->setCollisionState(true);
	} else if ((mCollisionCounter <= 1) && (hovercraft->isInCollisionState())) {
		hovercraft->setCollisionState(false);
	}
	//##End collision update

	//get current speed
	float currentspeed = mEntity->getSpeed();
	
	const BasicEntityEvent& status = hovercraft->getMovingStatus();

	if ( status.reset() ){

		int chechptn = mEntity->getRaceState()->getPlayer(mEntity->getPlayerId())->getLastCheckpoint();
		
		//std::cout << "PTN: " << chechptn << std::endl;

		Ogre::Vector3 pos;
		if ( chechptn == -1 ){
			std::vector<Entity*> startpoints = EntityManager::getServerSingletonPtr()->getEntities(StartPosition::CATEGORY);
			pos = startpoints[0]->getPosition();
		}
		else {
			//reset the damn thing
			std::vector<Entity*> resets = EntityManager::getServerSingletonPtr()->getEntities(ResetSpawn::CATEGORY);
			if ( chechptn >= resets.size() ){
				//finished -> allow reset to start
				std::vector<Entity*> startpoints = EntityManager::getServerSingletonPtr()->getEntities(StartPosition::CATEGORY);
				pos = startpoints[0]->getPosition();
			}
			else {
				//normal case, find best reset that has been reached
				Entity* closest = 0;
				int delta = 10000;	//big

				//find best
				for ( std::vector<Entity*>::iterator i = resets.begin();  i != resets.end(); i++ ){
					//std::cout << "ResetSpawn: " << ((ResetSpawn*)(*i))->getCheckpointId() << std::endl;

					//std::cout << "Abs Delta: " << abs(((ResetSpawn*)(*i))->getCheckpointId() - chechptn) << std::endl;

					int testdelta = abs(((ResetSpawn*)(*i))->getCheckpointId() - chechptn);

					if ( testdelta < delta ){
						closest = *i;
						delta = testdelta;
					}
				}

				pos = closest->getPosition();
			}
		}

		hkVector4 resetpos (pos[0],pos[1],pos[2]);
		mCharacterRigidBody->getRigidBody()->setPosition(resetpos);	
	}


	//Speed on a scale [0-1]
	float scaledspeed = 0.f;

	//speeding
	if (status.moveForward()) {	
		scaledspeed = 1;
	}

	//braking..
	if (status.moveBackward()) {
		scaledspeed = -1;	
	}

	//rotations
	if (status.moveLeft() || status.moveRight()) {

		// we can only turn while we're moving
		//if ((currentspeed > 1.0f) || (currentspeed < -1.0f)) {
			float angle = mCharacterRigidBody->getRigidBody()->getRotation().getAngle();
			if (angle != 0.0f) {
				hkVector4 axis = mUp;
				hkVector4 crossProduct;
				crossProduct.setCross(mUp, mSide);

				if (crossProduct(1) < 0) {
					angle = angle * -1;
				}
			}

			if (status.moveLeft()) {
				/*
				if (status.moveBackward()) {
					angle = -mRotationDelta;
				} else {
					angle = mRotationDelta;
				}
				*/
				
				if (currentspeed >= 0.0f) {
					angle = mRotationDelta;
				} else if (currentspeed < 0.0f) {
					angle = -mRotationDelta;
				}
				
			}
			if (status.moveRight()) {
				/*
				if (status.moveBackward()) {
					angle = mRotationDelta;
				} else {
					angle = -mRotationDelta;
				}
				*/
				
				if (currentspeed >= 0.0f) {
					angle = -mRotationDelta;
				} else if (currentspeed < 0.0f) {
					angle = mRotationDelta;
				}
				
			}


			hkQuaternion q(mUp,angle);
			mSide.setRotatedDir(q, mSide);
			mSide.normalize3();
		//}
	}

	hkRotation newOrientation;
	newOrientation.getColumn(0) = mSide;
	newOrientation.getColumn(1) = mUp;
	newOrientation.getColumn(2).setCross(mSide, mUp);
	newOrientation.renormalize();


	hkStepInfo stepInfo;
	stepInfo.m_deltaTime = Havok::getSingleton().getTimeStep();
	stepInfo.m_invDeltaTime = 1.0f / Havok::getSingleton().getTimeStep();

	hkpCharacterInput input;
	hkpCharacterOutput output;
	{
		HK_TIMER_BEGIN("set character state", HK_NULL);

		input.m_atLadder = false;
		input.m_inputLR = 0;
		input.m_inputUD = scaledspeed;
		input.m_wantJump = false;

		input.m_up = mUp;
		input.m_forward = newOrientation.getColumn(2);
		input.m_stepInfo = stepInfo;
		input.m_characterGravity.setMul4(-mCharacterGravity, mUp);
		input.m_velocity = mCharacterRigidBody->getRigidBody()->getLinearVelocity();
		input.m_position = mCharacterRigidBody->getRigidBody()->getPosition();
		mCharacterRigidBody->checkSupport(stepInfo, input.m_surfaceInfo);

		HK_TIMER_END();
	}

	// Apply the character state machine
	{
		HK_TIMER_BEGIN("update character state", HK_NULL);
		mCharacterContext->update(input, output);
		HK_TIMER_END();
	}

	//Apply the player character controller
	{
		HK_TIMER_BEGIN("simulate character", HK_NULL);
		// Set output velocity from state machine into character rigid body
		mCharacterRigidBody->setLinearVelocity(output.m_velocity, Havok::getSingleton().getTimeStep());
		HK_TIMER_END();
	}

	hkVector4 newSpeed = mCharacterRigidBody->getRigidBody()->getMotion()->getLinearVelocity();
	float actualSpeed = newSpeed.length3();
	// limit speed
	if (actualSpeed > hovercraft->getMaximumSpeed()) {
		newSpeed.normalize3();
		newSpeed.mul4(hovercraft->getMaximumSpeed());
		actualSpeed = newSpeed.length3();
	}
	
	//set new speed
	mEntity->setSpeed(actualSpeed);
	
	const hkReal gain = 0.25f;
	const hkQuaternion& currentOrient = mCharacterRigidBody->getRigidBody()->getRotation();
	hkQuaternion desiredOrient;
	desiredOrient.set(newOrientation);
	hkVector4 angle;
	hkVector4 angularVelocity;
	currentOrient.estimateAngleTo(desiredOrient, angle);
	angularVelocity.setMul4(gain / Havok::getSingleton().getTimeStep(), angle);
	mCharacterRigidBody->setAngularVelocity(angularVelocity);

	mWorld->unmarkForWrite();
}


void HavokHovercraft::load(const hkVector4& position, const hkQuaternion& rotation ){
	mWorld->markForWrite();

	//Hovercrafts right
	Hovercraft * hovercraft = dynamic_cast<Hovercraft*>(mEntity);

	//load the hovercraft hull from file

	hkIstream infile( mFilename.cString() );
	HK_ASSERT( 0x215d080c, infile.isOk() );

	/**
	 * The world in memory as loaded from file
	 */
	mPhysicsData = hkpHavokSnapshot::load( infile.getStreamReader(), &mLoadedData );
	HK_ASSERT( 0, mPhysicsData != HK_NULL );

	hkpRigidBody * body = mPhysicsData->findRigidBodyByName( mEntityName.cString() );
	HK_ASSERT( 0x215d080c, body != HK_NULL );

	const hkpShape * shape = body->getCollidable()->getShape();
	shape->addReference();

	//create a temporart shape
	hkpShape * tmp = new hkpSphereShape (5);

	//set up its parameters
	hkpCharacterRigidBodyCinfo info;
	info.m_mass = hovercraft->getMass();
	info.m_shape = tmp;
	info.m_maxForce = 1000.0f;
	info.m_position = position;
	info.m_maxLinearVelocity = Hovercraft::MAXSPEED;
	info.m_maxSpeedForSimplexSolver = Hovercraft::MAXSPEED;
	info.m_maxSlope = 45.0f * HK_REAL_DEG_TO_RAD;
	info.m_friction = 0.1f;
	info.m_rotation = rotation;

	//set up the actions
	hkpCharacterStateManager* manager = new hkpCharacterStateManager();

	{
		hkpCharacterStateOnGround *state = new hkpCharacterStateOnGround();
		manager->registerState( state,	HK_CHARACTER_ON_GROUND );
		state->setDisableHorizontalProjection( true );
		state->setSpeed(hovercraft->getMaximumSpeed());
		state->setGain(hovercraft->getAcceleration());
		state->setMaxLinearAcceleration(hovercraft->getAcceleration() * 10.0f);
		state->removeReference();
	}

	{
		hkpCharacterStateInAir * state = new hkpCharacterStateInAir();
		manager->registerState( state,	HK_CHARACTER_IN_AIR );
		state->setMaxLinearAcceleration(hovercraft->getAcceleration()*10);
		state->setGain(hovercraft->getAcceleration());
		state->setSpeed(hovercraft->getMaximumSpeed());
		state->removeReference();
	}

	{
		hkpCharacterStateJumping * state = new hkpCharacterStateJumping();
		manager->registerState( state,	HK_CHARACTER_JUMPING );
		state->removeReference();
	}

	mCharacterContext = new hkpCharacterContext( manager, HK_CHARACTER_IN_AIR );
	mCharacterContext->setCharacterType( hkpCharacterContext::HK_CHARACTER_RIGIDBODY );
	mCharacterContext->setFilterEnable(false);

	//Create a character rigid body
	mCharacterRigidBody = new hkpCharacterRigidBody( info );
	hkpCharacterRigidBodyListener* listener = new hkpCharacterRigidBodyListener();
	mCharacterRigidBody->setListener( listener );

	listener->removeReference();		
	hkpRigidBody * charbody = mCharacterRigidBody->getRigidBody();

	//set the loaded shape here
	charbody->setShape(shape);

	//set name and type
	charbody->setUserData(reinterpret_cast<hkUlong>(this));
	HavokEntityType::setEntityType(charbody,HavokEntityType::CHARACTER);
	mWorld->addEntity( charbody );

	//collision prevention test
	//mCollisionTest = new SimpleTest(mWorld,this);
	mCollisionTest = new AdvancedTest(mWorld,this);

	//Hover action
	mWorld->addAction(new HoverAction(this,mWorld));
	mWorld->addAction(new HavokHovercraftCollisionEffect(this));
	mWorld->addAction(new UpdatePositionAction(charbody,mEntity));

	//Remove some data
	tmp->removeReference();
	//charbody->removeReference();

	mWorld->unmarkForWrite();
}

void HavokHovercraft::decreaseCollisionCounter() {
	mCollisionCounter--;
}

void HavokHovercraft::increaseCollisionCounter() {
	mCollisionCounter++;
}

}