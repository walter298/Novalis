#pragma once

//#include "Node.h"

#include "detail/memory/MemoryPool.h"
#include "ResourceRegistry.h"

namespace nv {
	class Instance {
	private:
		int m_screenWidth  = 0;
		int m_screenHeight = 0;

		struct Guard {
			SDL_Window* window     = nullptr;
			SDL_Renderer* renderer = nullptr;

			void quit();
			~Guard();
		};
		/*IMPORTANT: guard is declared after registry because we want registry to be destroyed first. 
		registry contains texture resources, and those must be destroyed before we call SDL_Quit(),
		otherwise we might get read access violations*/
		Guard m_guard; 
	public:
		ResourceRegistry registry;
		
		Instance(const char* windowTitle) noexcept;
		Instance(Instance&&) = delete;
		
		int getScreenWidth() const noexcept;
		int getScreenHeight() const noexcept;
		SDL_Renderer* getRenderer() noexcept;
		SDL_Window* getWindow() noexcept;
	};

	Instance* getGlobalInstance() noexcept;
}
