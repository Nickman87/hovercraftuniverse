#pragma once

#include "NetworkEntity.h"
#include <zoidcom/zoidcom_control.h>

class Tree : public HovUni::NetworkEntity
{
	static ZCom_ClassID mClassID;
public:
	Tree(ZCom_Control* control);
	~Tree(void);

	virtual void parseEvents(ZCom_BitStream* stream);

	static void registerClass(ZCom_Control* control);
	static ZCom_ClassID getClassID();
};
