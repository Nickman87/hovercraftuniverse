#include "StdAfx.h"
#include "SampleEventMoveForward.h"
#include "SampleEventMoveBackward.h"
#include "SampleEventMoveLeft.h"
#include "SampleEventMoveRight.h"
#include "SampleEventParser.h"

namespace HovUni {

SampleEventParser::~SampleEventParser() {

}

NetworkEvent<SampleEventType>* SampleEventParser::parse(ZCom_BitStream* stream) {
	SampleEventType type = NetworkEvent<SampleEventType>::readType(stream);
	
	switch (type) {
		case MoveForward:
			return SampleEventMoveForward::parse(stream);
			break;
		case MoveBackward:
			return SampleEventMoveBackward::parse(stream);
			break;
		case MoveLeft:
			return SampleEventMoveLeft::parse(stream);
			break;
		case MoveRight:
			return SampleEventMoveRight::parse(stream);
			break;
		default:
			return new SampleEventMoveForward(1.5);
			break;
	}
}

}