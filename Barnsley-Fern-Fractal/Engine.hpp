#ifndef ENGINE_HPP
#define ENGINE_HPP

struct SDL_Texture;
struct SDL_Color;

#include <SDL_ttf.h>

class Engine {
public:
	static Engine* get_instance();
	~Engine();

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	void main_loop();

private:
	Engine();

	void reset();
	void capture_screenshot() const;

	void handle_input();
	void update();
	void render() const;

	double prev_x, prev_y;
	unsigned total_iters = 0;	//total iterations of algorithm
	unsigned changed_iters = 0;	//iterations during which a new pixel was set, these are the 'useful' iterations
	unsigned iters_per_frame = 1;	//number of times to run algorithm per frame

	bool running;
	SDL_Texture* win_texture;
	TTF_Font* font;


	/*
	* to store screen state
	* we do this because locking/unlocking GPU texture is expensive
	* so we store a separate screen buffer and avoid redundant locking
	* of texture whenever possible
	*/
	bool* screen_pixels;
};

#endif // !ENGINE_HPP
