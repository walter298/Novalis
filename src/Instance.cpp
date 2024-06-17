#include "Instance.h"

void nv::Instance::quit() {
	m_textures.clear(); //it's important to clear the map because SDL must be shut down AFTER the textures are destroyed
	
	IMG_Quit();
	Mix_Quit();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

nv::Instance::Instance(std::string windowTitle)
	: window{ SDL_CreateWindow(windowTitle.c_str(), 0, 0, NV_SCREEN_WIDTH, NV_SCREEN_HEIGHT, SDL_WINDOW_OPENGL) },
	renderer{ SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED) }
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0 || //returns zero on sucess
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0 ||
		//TTF_Init() != 0 || 
		IMG_Init(IMG_INIT_JPG & IMG_INIT_PNG) != 0)
	{
		quit();
		std::cerr << SDL_GetError() << '\n';
		exit(-1);
	}
	nv::workingDirectory(); //set working directory
	//Text::openFonts();
}

nv::Instance::~Instance() {
	std::puts("Quitting\n");
	quit();
}

const std::vector<nv::Texture>& nv::Instance::getTextures(const std::string& key) const {
	return m_textures.at(key);
}

SDL_Window* nv::Instance::getRawWindow() noexcept {
	return window;
}

SDL_Renderer* nv::Instance::getRawRenderer() noexcept {
	return renderer;
}

//void nv::Instance::loadObjsFromDir(std::string absDirPath) {
//	using namespace std::filesystem;
//	if (!exists(absDirPath)) {
//		throw std::runtime_error("Error: " + absDirPath + " does not exist.\n");
//	}
//
//	std::vector<std::string> subDirectoryPaths; //file paths of directories within the current directory
//
//	for (const auto& entry : directory_iterator(absDirPath)) {
//		auto currPath = entry.path().string();
//		std::ranges::replace(currPath, '\\', '/');
//		
//		if (!entry.is_directory()) {
//			auto fileExt = fileExtension(currPath);
//			if (fileExt) {
//				auto& ext = *fileExt;
//				if (ext == ".nv_sprite") {
//					/*Sprite sprite{ m_SDLRenderer, currPath };
//					m_spriteMap[sprite.getName()] = std::move(sprite);*/
//				} 
//			}
//		} else {
//			//subDirectoryPaths.push_back(std::move(currentPath));
//		}
//	}
//
//	for (const auto& nestedPath : subDirectoryPaths) {
//		loadObjsFromDir(nestedPath);
//	}
//}