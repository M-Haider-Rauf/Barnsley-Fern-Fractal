#include "Functions.hpp"

#include <random>

//'maps' a given range onto another
//analogous to stretching and/or shifting a function
double map(double val, double in_min, double in_max, double out_min, double out_max)
{
	return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void next_point(double prev_x, double prev_y, double* out_x, double* out_y)
{

	static std::uniform_real_distribution<double> dist(0, 1.0);
	static std::default_random_engine eng(std::random_device().operator()());

	double chance = dist(eng);

	//the 'scale matrixes for functions'
	static const double scales[4][4] = {
		{ 0.0, 0.0, 0.0, 0.16     },     //f1
		{ 0.85, 0.04, -0.04, 0.85 },     //f2
		{ 0.20, -0.26, 0.23, 0.22 },     //f3
		{ -0.15, 0.28, 0.26, 0.24 }      //f4
	};

	//the 'schift matrixes for functions'
	static const double shifts[4][2] = {
		{ 0.0, 0.0  },	//f1
		{ 0.0, 1.60 },	//f2
		{ 0.0, 1.60 },  //f3
		{ 0.0, 0.44 }   //f4
	};

	//the probabilities of choosing a given function
	static const double p[4] = { 0.01, 0.85, 0.07, 0.07 };
	
	//f1
	if (chance < p[0]) {
		*out_x = scales[0][0] * prev_x + scales[0][1] * prev_y + shifts[0][0];
		*out_y = scales[0][2] * prev_x + scales[0][3] * prev_y + shifts[0][1];
	}

	//f2
	else if (chance < (p[0] + p[1])) {
		*out_x = scales[1][0] * prev_x + scales[1][1] * prev_y + shifts[1][0];
		*out_y = scales[1][2] * prev_x + scales[1][3] * prev_y + shifts[1][1];
	}

	//f3
	else if (chance < (p[0] + p[1] + p[2])) {
		*out_x = scales[2][0] * prev_x + scales[2][1] * prev_y + shifts[2][0];
		*out_y = scales[2][2] * prev_x + scales[2][3] * prev_y + shifts[2][1];
	}

	//f4
	else  {
		*out_x = scales[3][0] * prev_x + scales[3][1] * prev_y + shifts[3][0];
		*out_y = scales[3][2] * prev_x + scales[3][3] * prev_y + shifts[3][1];
	}

}
