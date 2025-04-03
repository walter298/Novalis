#include "Instance.h"

#include <print>
#include <SDL3_image/SDL_image.h>
//#include <SDL3_ttf/SDL_ttf.h>

#include "detail/file/File.h"

void nv::Instance::quit() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	//TTF_Quit();
	//IMG_Quit();
	/*Mix_Quit();
	TTF_Quit();*/
	SDL_Quit();
}

static nv::Instance* globalInstance = nullptr;

nv::Instance::Instance(const char* windowTitle) noexcept {
	auto exitWithError = [this] {
		std::println("{}", SDL_GetError());
		quit();
		exit(-1);
	};

	if (!(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO))) {
		exitWithError();
	}

	//if (SDL_Init(SDL_INIT_EVERYTHING) != 0 || //returns zero on sucess
	//	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0 ||
	//	TTF_Init() != 0 || 
	//	IMG_Init(IMG_INIT_PNG) == 0)
	//{
	//	exitWithError();
	//}

	//get screen width and height
	auto dm = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
	m_screenWidth = dm->w;
	m_screenHeight = dm->h;

	window = SDL_CreateWindow(windowTitle, m_screenWidth, m_screenHeight, SDL_WINDOW_OPENGL);
	if (window == nullptr) {
		exitWithError();
	}
	renderer = SDL_CreateRenderer(window, nullptr);
	if (renderer == nullptr) {
		exitWithError();
	}

	SDL_SetRenderLogicalPresentation(renderer, m_screenWidth, m_screenHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	workingDirectory(); //initialize local static inside workingDirectory

	globalInstance = this;
}

nv::Instance::~Instance() noexcept {
	quit();
}

int nv::Instance::getScreenWidth() const noexcept {
	return m_screenWidth;
}

int nv::Instance::getScreenHeight() const noexcept {
	return m_screenHeight;
}

nv::Instance* nv::getGlobalInstance() noexcept {
	return globalInstance;
}
