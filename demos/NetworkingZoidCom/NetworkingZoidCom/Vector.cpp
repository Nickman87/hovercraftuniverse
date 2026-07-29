#include "StdAfx.h"
#include "Vector.h"

Vector::Vector() : mX(0), mY(0), mZ(0) {

}

Vector::Vector(double x, double y, double z) : mX(x), mY(y), mZ(z) {

}

Vector::~Vector() {

}

double Vector::getX() const {
	return mX;
}

double Vector::getY() const {
	return mY;
}

double Vector::getZ() const {
	return mZ;
}

void Vector::setX(double x) {
	mX = x;
}

void Vector::setY(double y) {
	mY = y;
}

void Vector::setZ(double z) {
	mZ = z;
}