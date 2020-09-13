#include "Engine.hpp"

/*
	Barnsley Fern Fractal plotter
	written by Haider Rauf
	6 July, 2020
*/


int main(int argc, char** args)
{
	Engine* engine = Engine::get_instance();
	engine->main_loop();

	return 0;
}