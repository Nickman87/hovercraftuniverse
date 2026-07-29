#include "StdAfx.h"
#include "Tree.h"

ZCom_ClassID Tree::mClassID = ZCom_Invalid_ID;

Tree::Tree(ZCom_Control* control) {
	mNode->registerNodeDynamic(mClassID, control);
}

Tree::~Tree(void) {

}

void Tree::parseEvents(ZCom_BitStream* stream) {
	// Do something
}

void Tree::registerClass(ZCom_Control* control) {
	mClassID = control->ZCom_registerClass("Tree");
}

ZCom_ClassID Tree::getClassID() {
	return mClassID;
}