#include "OgreLuaBindings.h"

// Prototype this before operator.hpp so it can be found for tostring() operator.
std::ostream& operator<<(std::ostream& stream, const Ogre::Entity& ent);

#include "luabind/operator.hpp"

using namespace Ogre;
using namespace luabind;

std::ostream& operator<<(std::ostream& stream, const Ogre::Entity& ent) {
	return stream << ent.getName();
}


namespace HovUni {
	OgreLuaBindings::OgreLuaBindings(lua_State* L) {
		mLuaState = L;
	}

	OgreLuaBindings::~OgreLuaBindings() {
		//...
	}

	#define LUA_CONST_START(class) { object g = globals(L); object table = g[#class];
	#define LUA_CONST(class, name) table[#name] = class::name
	#define LUA_CONST_END }

	void OgreLuaBindings::bindVector3()	{
		lua_State* L = mLuaState;
		module(L)
		[
			class_<Vector3>("Vector3")
			.def(tostring(self))
			.def_readwrite("x", &Vector3::x)
			.def_readwrite("y", &Vector3::y)
			.def_readwrite("z", &Vector3::z)
			.def(constructor<>())
			.def(constructor<Vector3&>())
			.def(constructor<Real, Real, Real>())
			.def("absDotProduct", &Vector3::absDotProduct)
			.def("crossProduct", &Vector3::crossProduct)
			.def("directionEquals", &Vector3::directionEquals)
			.def("distance", &Vector3::distance)
			.def("dotProduct", &Vector3::dotProduct)
			.def("getRotationTo", &Vector3::getRotationTo)
			.def("isZeroLength", &Vector3::isZeroLength)
			.def("length", &Vector3::length)
			.def("makeCeil", &Vector3::makeCeil)
			.def("makeFloor", &Vector3::makeFloor)
			.def("midPoint", &Vector3::midPoint)
			.def("normalise", &Vector3::normalise)
			.def("normalisedCopy", &Vector3::normalisedCopy)
			.def("perpendicular", &Vector3::perpendicular)
			.def("positionCloses", &Vector3::positionCloses)
			.def("positionEquals", &Vector3::positionEquals)
			//.def("ptr", &Vector3::ptr)
			.def("randomDeviant", &Vector3::randomDeviant)
			.def("reflect", &Vector3::reflect)
			.def("squaredDistance", &Vector3::squaredDistance)
			.def("squaredLength", &Vector3::squaredLength)

			// Operators

			.def(self + other<Vector3>())
			.def(self - other<Vector3>())
			.def(self * other<Vector3>())
			.def(self * Real())
		];

		LUA_CONST_START(Vector3)
			LUA_CONST(Vector3, ZERO);
			LUA_CONST(Vector3, UNIT_X);
			LUA_CONST(Vector3, UNIT_X);
			LUA_CONST(Vector3, UNIT_Y);
			LUA_CONST(Vector3, UNIT_Z);
			LUA_CONST(Vector3, NEGATIVE_UNIT_X);
			LUA_CONST(Vector3, NEGATIVE_UNIT_Y);
			LUA_CONST(Vector3, NEGATIVE_UNIT_Z);
			LUA_CONST(Vector3, UNIT_SCALE);
		LUA_CONST_END;
	}

	void OgreLuaBindings::bindVector4()	{
		lua_State* L = mLuaState;
		module(L)
		[
			class_<Vector4>("Vector4")
			.def(tostring(self))
			.def_readwrite("w", &Vector4::w)
			.def_readwrite("x", &Vector4::x)
			.def_readwrite("y", &Vector4::y)
			.def_readwrite("z", &Vector4::z)
			.def(constructor<>())
			.def(constructor<Vector4&>())
			.def(constructor<Real, Real, Real, Real>())
			.def(self + other<Vector4>())
			.def(self - other<Vector4>())
			.def(self * other<Vector4>())
		];
	}

	void OgreLuaBindings::bindColourValue()	{
		lua_State* L = mLuaState;
		module(L)
		[
			class_<ColourValue>("ColourValue")
			.def(constructor<>())
			.def(constructor<Real, Real, Real, Real>())
			.def(constructor<Real, Real, Real>())
			.def(tostring(self))
			.def_readwrite("r", &ColourValue::r)
			.def_readwrite("g", &ColourValue::g)
			.def_readwrite("b", &ColourValue::b)
			.def_readwrite("a", &ColourValue::a)
			.def("saturate", &ColourValue::saturate)

			// Operators

			.def(self + other<ColourValue>())
			.def(self - other<ColourValue>())
			.def(self * other<ColourValue>())
			.def(self * Real())
			.def(self / Real())
		];

		LUA_CONST_START(ColourValue)
			LUA_CONST(ColourValue, ZERO);
			LUA_CONST(ColourValue, Black);
			LUA_CONST(ColourValue, White);
			LUA_CONST(ColourValue, Red);
			LUA_CONST(ColourValue, Green);
			LUA_CONST(ColourValue, Blue);
		LUA_CONST_END;
	}

	void OgreLuaBindings::bindEntity() {
		lua_State* L = mLuaState;
		module(L)
		[
			class_<MovableObject>("MovableObject"),
			class_<Entity, MovableObject>("Entity")
			.def(tostring(self))
			.def("setMaterialName", &Entity::setMaterialName)
			.def("setDisplaySkeleton", &Entity::setDisplaySkeleton)
		];
	}

	void OgreLuaBindings::bindCamera() {
		lua_State* L = mLuaState;
		module(L)
		[
			class_<Camera>("Camera")
			.def("setPosition", (void(Camera::*)(const Vector3&))&Camera::setPosition)
			.def("setPosition", (void(Camera::*)(Real,Real,Real))&Camera::setPosition)
			.def("lookAt", (void(Camera::*)(const Vector3&))&Camera::lookAt)
			.def("lookAt", (void(Camera::*)(Real,Real,Real))&Camera::lookAt)
			.def("setNearClipDistance", &Camera::setNearClipDistance)
			.def("setFarClipDistance", &Camera::setFarClipDistance)
		];
	}

	void OgreLuaBindings::bindLua() {
		bindVector3();
		bindVector4();
		bindColourValue();
		bindEntity();
		bindCamera();
	}
}