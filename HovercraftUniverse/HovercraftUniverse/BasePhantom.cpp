//Havok
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>

#include "BasePhantom.h"

namespace HovUni {

BasePhantom::BasePhantom( const hkpShape *shape, const hkTransform &transform ):
	hkpSimpleShapePhantom(shape,transform)
{
}

BasePhantom::~BasePhantom(void)
{
}

}
