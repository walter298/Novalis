#pragma once

//#include "Node.h"

#include "detail/memory/MemoryPool.h"
#include "ResourceRegistry.h"

namespace nv {
	class Instance {
	private:
		void quit();

		int m_screenWidth  = 0;
		int m_screenHeight = 0;
	public:
		SDL_Window* window     = nullptr;
		SDL_Renderer* renderer = nullptr;
		ResourceRegistry registry;
		
		Instance(const char* windowTitle) noexcept;
		Instance(Instance&&) = delete;
		~Instance() noexcept;

		int getScreenWidth() const noexcept;
		int getScreenHeight() const noexcept;
	};

	Instance* getGlobalInstance() noexcept;
}
