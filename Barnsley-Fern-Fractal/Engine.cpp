#include "Engine.hpp"
#include "Window.hpp"
#include "Constants.hpp"
#include "Functions.hpp"

#include <SDL.h>
#include <SDL_ttf.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <ctime>

//a macro for force termination and printing to console,,,
#define ERR_ABORT(STR) do { std::cerr << STR << '\n'; std::abort(); } while (0)

Engine* Engine::get_instance()
{
	static Engine engine;
	return &engine;
}

Engine::Engine() 
	: running(true)
	, prev_x(0.0)
	, prev_y(0.0)
	, screen_pixels(nullptr)
	, font (nullptr)
{
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();

	g_window = SDL_CreateWindow(
		"Haider Rauf",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WIN_WIDTH, WIN_HEIGHT,
		SDL_WINDOW_SHOWN
		);

	if (!g_window) {
		ERR_ABORT("Error creating window!");
	}

	g_renderer = SDL_CreateRenderer(
		g_window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
		);

	if (!g_renderer) {
		ERR_ABORT("Error creating renderer!");
	}

	g_rend_info = new SDL_RendererInfo;
	SDL_GetRendererInfo(g_renderer, g_rend_info);

	win_texture = SDL_CreateTexture(
		g_renderer,
		g_rend_info->texture_formats[0],
		SDL_TEXTUREACCESS_STREAMING,
		WIN_WIDTH, WIN_HEIGHT
		);

	if (!win_texture) {
		ERR_ABORT("Error creating window texture!");
	}

	font = TTF_OpenFont("font.ttf", 18);
	if (!font) {
		ERR_ABORT("Error creating font!");
	}

	screen_pixels = new bool[WIN_WIDTH * WIN_HEIGHT]{ 0 }; 
}

void Engine::reset()
{
	changed_iters = total_iters = prev_x = prev_y = 0.0;
	//reset offscreen buffer
	std::memset(screen_pixels, 0, sizeof * screen_pixels * WIN_WIDTH * WIN_HEIGHT);

	void* pixels = nullptr;
	int pitch = 0;

	//clear texture to BLACK
	SDL_LockTexture(win_texture, nullptr, &pixels, &pitch);
	std::memset(pixels, 0, pitch * WIN_HEIGHT);
	SDL_UnlockTexture(win_texture);
}

void Engine::capture_screenshot() const
{
	char file_name[20];

	SDL_Surface* ss = SDL_CreateRGBSurfaceWithFormat(0, WIN_WIDTH, WIN_HEIGHT, 32, g_rend_info->texture_formats[0]);
	SDL_RenderReadPixels(g_renderer, nullptr, g_rend_info->texture_formats[0], ss->pixels, ss->pitch);

	sprintf_s(file_name, "ss_%lld.bmp", std::time(nullptr));
	SDL_SaveBMP(ss, file_name);

	SDL_FreeSurface(ss);
}

void Engine::main_loop()
{
	while (running) {
		handle_input();
		update();
		render();
	}
}

void Engine::handle_input()
{
	SDL_Event event_q{};
	while (SDL_PollEvent(&event_q)) {
		if (event_q.type == SDL_QUIT) {
			running = false;
		}
		else if (event_q.type == SDL_KEYDOWN) {
			switch (event_q.key.keysym.sym) {
			case SDLK_ESCAPE:
				running = false;
				break;

			case SDLK_c:
				this->capture_screenshot();
				break;

			case SDLK_UP:
				if (iters_per_frame < MAX_ITERS) iters_per_frame *= 2;
				break;

			case SDLK_DOWN:
				if (iters_per_frame > MIN_ITERS) iters_per_frame /= 2;
				break;

			case SDLK_r:
				this->reset();
				break;


			default:
				break;
			}
		}
	}
}

void Engine::update()
{

	static const SDL_PixelFormat* PIXEL_FORMAT = SDL_AllocFormat(g_rend_info->texture_formats[0]);
	static const Uint32 FRAC_COLOR = SDL_MapRGB(PIXEL_FORMAT, 124, 252, 0); //color of fractal (LAWN GREEN)


	for (unsigned i = 0; i < iters_per_frame; ++i) {
		double curr_x, curr_y;

		next_point(prev_x, prev_y, &curr_x, &curr_y); //get next point of fractal

		//convert to pixel space
		int pixel_y = std::round(map(curr_x, -2.2, 2.7, WIN_HEIGHT - 1, 0));
		int pixel_x = std::round(map(curr_y, 0.0, 10.0, 0, WIN_WIDTH - 1));

		bool& target_pixel = screen_pixels[pixel_y * WIN_WIDTH + pixel_x];

		//only bother with updating texture, if this screen pixel is NOT SET
		//done to avoid expensive locking / unlocking of GPU texture
		if (!target_pixel) {

			void* pixels = nullptr;
			int pitch;

			SDL_LockTexture(win_texture, nullptr, &pixels, &pitch);


			const int offset = pixel_y * pitch + pixel_x * 4;
			Uint32* target = (Uint32*)((Uint8*)pixels + offset);
			*target = FRAC_COLOR;

			SDL_UnlockTexture(win_texture);

			target_pixel = true;
			++changed_iters;
		}

		prev_x = curr_x;
		prev_y = curr_y;

		++total_iters;
	}
}

void Engine::render() const
{
	//clear screen
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
	SDL_RenderClear(g_renderer);

	//blit texture
	SDL_RenderCopy(g_renderer, win_texture, nullptr, nullptr);

	/*draw info text
	  A bit inefficient since we're allocating / deallocatingfont textures 
	  every frame, but doesn't matter because C++ is fast as fuck anyway*/
	//**************************************************************
	std::ostringstream sout;
	sout << "Total iterations: " << std::left << std::setw(10) << total_iters;
	sout << "Update iterations: " << std::left << std::setw(8) << changed_iters;
	sout << "Percentage: " << std::left << std::setw(8) << (100.0 * changed_iters / total_iters);
	sout << "(at " << iters_per_frame << " IPF)";
	sout << std::right;
	
	SDL_Surface* text_surf = TTF_RenderText_Solid(font, sout.str().c_str(), { 255, 0, 0, 0 });
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(g_renderer, text_surf);

	SDL_Rect text_rect = { 0, 0, text_surf->w, text_surf->h };

	SDL_RenderCopy(g_renderer, text_texture, nullptr, &text_rect);
		 
	//***********************************************************
	SDL_RenderPresent(g_renderer);
	
	SDL_FreeSurface(text_surf);
	SDL_DestroyTexture(text_texture);
}

Engine::~Engine()
{
	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_DestroyTexture(win_texture);
	TTF_CloseFont(font);
	
	delete g_rend_info;
	delete[] screen_pixels;

	SDL_Quit();
}

