#include "StdAfx.h"
#include "NetworkEvent.h"
#include "Player.h"
#include "SampleEventMoveBackward.h"
#include "SampleEventMoveForward.h"
#include "SampleEventMoveLeft.h"
#include "SampleEventMoveRight.h"
#include "SampleEventParser.h"

ZCom_ClassID Player::mClassID = ZCom_Invalid_ID;

Player::Player(ZCom_Control* control) {
	setupReplication();
	mNode->registerNodeDynamic(mClassID, control);
}

Player::~Player(void) {

}

void Player::parseEvents(ZCom_BitStream* stream) {
	// Parse
	HovUni::SampleEventParser p;
	HovUni::NetworkEvent<SampleEventType>* event = p.parse(stream);

	// Process

	// Server
	if (mNode->getRole() == eZCom_RoleAuthority) {
		switch (event->getType()) {
			case HovUni::MoveForward:
				{
					cout << "Player is trying to move forward" << endl;
					// Let's allow the movement
					HovUni::SampleEventMoveForward* e = dynamic_cast<HovUni::SampleEventMoveForward*>(event);
					if (e) {
						mPosition.setX(mPosition.getX() + e->getDistance());
						sendEvent(*event);
					}
				}
				break;
			case HovUni::MoveBackward:
				{
					cout << "Player is trying to move backward" << endl;
					// Let's allow the movement
					HovUni::SampleEventMoveBackward* e = dynamic_cast<HovUni::SampleEventMoveBackward*>(event);
					if (e) {
						mPosition.setX(mPosition.getX() - e->getDistance());
						sendEvent(*event);
					}
				}
				break;
			case HovUni::MoveLeft:
				{
					cout << "Player is trying to move left" << endl;
					// Let's allow the movement
					HovUni::SampleEventMoveLeft* e = dynamic_cast<HovUni::SampleEventMoveLeft*>(event);
					if (e) {
						mPosition.setZ(mPosition.getZ() - e->getDistance());
						sendEvent(*event);
					}
				}
				break;
			case HovUni::MoveRight:
				{
					cout << "Player is trying to move right but crashed" << endl;
					// Cannot allow movement so shock reaction to left
					HovUni::SampleEventMoveRight* e = dynamic_cast<HovUni::SampleEventMoveRight*>(event);
					if (e) {
						HovUni::SampleEventMoveLeft seml(e->getDistance() / 2.0f);
						mPosition.setZ(mPosition.getZ() - seml.getDistance());
						sendEvent(seml);
					}
				}
				break;

			default:
				break;
		}

	// Owner
	} else if (mNode->getRole() == eZCom_RoleOwner) {
		switch (event->getType()) {
			case HovUni::MoveForward:
				cout << "You moved forward" << endl;
				break;
			case HovUni::MoveBackward:
				cout << "You moved backward" << endl;
				break;
			case HovUni::MoveLeft:
				cout << "You moved left" << endl;
				break;
			case HovUni::MoveRight:
				cout << "You moved right" << endl;
				break;

			default:
				break;
		}
		cout << "Position: (" << mPosition.getX() << ", " << mPosition.getY() << ", " << mPosition.getZ() << ")" << endl;

	// Others
	} else {
		switch (event->getType()) {
			case HovUni::MoveForward:
				cout << "Someone moved forward" << endl;
				break;
			case HovUni::MoveBackward:
				cout << "Someone moved backward" << endl;
				break;
			case HovUni::MoveLeft:
				cout << "Someone moved left" << endl;
				break;
			case HovUni::MoveRight:
				cout << "Someone moved right" << endl;
				break;

			default:
				break;
		}
	}
}

void Player::setupReplication() {
	//mNode->beginReplicationSetup(3);

	//mNode->addReplicationFloat(&mX, 23, ZCOM_REPFLAG_MOSTRECENT, ZCOM_REPRULE_AUTH_2_ALL);
	//mNode->addReplicationFloat(&mY, 23, ZCOM_REPFLAG_MOSTRECENT, ZCOM_REPRULE_AUTH_2_ALL);
	//mNode->addReplicationFloat(&mZ, 23, ZCOM_REPFLAG_MOSTRECENT, ZCOM_REPRULE_AUTH_2_ALL);

	//mNode->endReplicationSetup();
}

void Player::registerClass(ZCom_Control* control) {
	mClassID = control->ZCom_registerClass("Player");
}

ZCom_ClassID Player::getClassID() {
	return mClassID;
}