#include "Instance.h"

#include <print>

void nv::Instance::quit() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	Mix_Quit();
	SDL_Quit();
}

SDL_Texture* nv::Instance::getTexture(std::string_view str) {
	auto texIt = m_textures.find(str.data());
	if (texIt != m_textures.end()) {
		return texIt->second.raw;
	} 
	auto newTex = IMG_LoadTexture(renderer, str.data());
	assert(newTex != nullptr);
	m_textures.emplace(str.data(), newTex);
	return newTex;
}

nv::Instance::Instance(std::string_view windowTitle) {
	auto exitWithError = [this] {
		std::println("{}", SDL_GetError());
		quit();
		exit(-1);
	};

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0 || //returns zero on sucess
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0 ||
		//TTF_Init() != 0 || 
		IMG_Init(IMG_INIT_PNG) == 0)
	{
		exitWithError();
	}

	window = SDL_CreateWindow(windowTitle.data(), 0, 0, NV_SCREEN_WIDTH, NV_SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
	if (window == nullptr) {
		exitWithError();
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr) {
		exitWithError();
	}

	SDL_RenderSetScale(renderer, static_cast<float>(NV_SCREEN_WIDTH / 1920), static_cast<float>(NV_SCREEN_HEIGHT / 1080));
}

nv::Instance::~Instance() noexcept {
	quit();
}