#pragma once

class Vector
{
public:
	Vector();
	Vector(double x, double y, double z);
	~Vector();
	double getX() const;
	double getY() const;
	double getZ() const;
	void setX(double x);
	void setY(double y);
	void setZ(double z);

private:
	double mX;
	double mY;
	double mZ;
};
