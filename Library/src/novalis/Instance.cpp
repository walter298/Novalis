#include "Instance.h"

#include <print>
#include <SDL3_image/SDL_image.h>
#include "detail/file/File.h"

static nv::Instance* globalInstance = nullptr;

nv::Instance::Instance(const char* windowTitle) noexcept {
	auto exitWithError = [this] {
		std::println("{}", SDL_GetError());
		exit(-1);
	};

	if (!(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO))) {
		exitWithError();
	}

	//get screen width and height
	auto dm = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
	m_screenWidth = dm->w;
	m_screenHeight = dm->h;

	m_guard.window = SDL_CreateWindow(windowTitle, m_screenWidth, m_screenHeight, SDL_WINDOW_OPENGL);
	if (m_guard.window == nullptr) {
		exitWithError();
	}
	m_guard.renderer = SDL_CreateRenderer(m_guard.window, nullptr);
	if (m_guard.renderer == nullptr) {
		exitWithError();
	}

	SDL_SetRenderLogicalPresentation(m_guard.renderer, m_screenWidth, m_screenHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	workingDirectory(); //initialize local static inside workingDirectory

	globalInstance = this;
}

int nv::Instance::getScreenWidth() const noexcept {
	return m_screenWidth;
}

int nv::Instance::getScreenHeight() const noexcept {
	return m_screenHeight;
}

SDL_Renderer* nv::Instance::getRenderer() noexcept {
	return m_guard.renderer;
}

SDL_Window* nv::Instance::getWindow() noexcept {
	return m_guard.window;
}

nv::Instance* nv::getGlobalInstance() noexcept {
	return globalInstance;
}

void nv::Instance::Guard::quit() {
	if (renderer && window) {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
}

nv::Instance::Guard::~Guard()
{
	quit();
}
