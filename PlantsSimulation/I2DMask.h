#pragma once
class I2DMask
{
public:
	// Refactor: added const — implementations only read spatial data, never mutate state.
	virtual double get2DMaskValue(double x, double z, int blur) const { return 0.9; }
};
