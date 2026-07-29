#ifndef BASEPHANTOM_H
#define BASEPHANTOM_H

#include <Physics/Dynamics/Phantom/hkpSimpleShapePhantom.h>

namespace HovUni {

class BasePhantom : public hkpSimpleShapePhantom
{
public:
	BasePhantom( const hkpShape *shape, const hkTransform &transform );

	~BasePhantom(void);

	//virtual void  addOverlappingCollidable (hkpCollidable *collidable);
 
	//virtual void  removeOverlappingCollidable (hkpCollidable *collidable);

};

}

#endif
