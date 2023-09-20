#pragma once
class I2DMask
{
public:
	virtual double get2DMaskValue(double x, double z, int blur) { return 0.9; };
};
