#pragma once

#include "NetworkEntity.h"
#include "NetworkEvent.h"
#include "SampleEventType.h"
#include "SampleEventMoveLeft.h"
#include "Vector.h"
#include <zoidcom/zoidcom_control.h>

using HovUni::SampleEventType;

class Player: public HovUni::NetworkEntity
{
	static ZCom_ClassID mClassID;
public:
	Player(ZCom_Control* control);
	~Player(void);

	virtual void parseEvents(ZCom_BitStream* stream);

	void setupReplication();

	static void registerClass(ZCom_Control* control);
	static ZCom_ClassID getClassID();

private:
	Vector mPosition;
};
