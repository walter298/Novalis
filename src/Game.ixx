#include <map>
#include <string>
#include <filesystem>
#include <iostream>
#include <type_traits>

#include "SDL.h"

import RenderObj;
import EventHandler;

export module Game;

namespace nv {
	export class Game {
	protected:
		inline static SDL_Window* m_window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
							                 NV_SCREEN_WIDTH, NV_SCREEN_HEIGHT, SDL_WINDOW_VULKAN);
		static inline SDL_Renderer* m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

		virtual void customRender() {};

		static inline int m_mouseX, m_mouseY;

		static inline std::vector<nv::InputEvent> m_inputEvents;
		static inline std::vector<nv::KeyboardEvent> m_keyboardEvents;
		static inline std::vector<nv::CustomEvent> m_customEvents;

		virtual void customInit() {};

		virtual void quit() {
			SDL_DestroyRenderer(m_renderer);
			SDL_DestroyWindow(m_window);

			SDL_Quit();

			exit(0);
		}
	public:
		class TextureStorage {
		private:
			static inline std::map<std::string, RenderObj> textureMap;
		public:
			static void push(std::string fullPath) {
				textureMap[fullPath] = RenderObj(m_renderer, fullPath);
			}

			static RenderObj get(std::string name) {
				return textureMap.at(name);
			}

			TextureStorage() = delete;
		};

		static void init() { //initializes SDL, loads in textures
			if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
				std::cout << "Error: SDL failed to initialize\n";
				exit(1);
			}

			using namespace std::filesystem;

			std::string path = NV_WORKING_DIRECTORY;
			path.append("static_objects/"); //because adding const chars* doesn't work

			for (const auto& entry : directory_iterator(path)) {
				std::cout << "loading " << entry.path().string() << std::endl;

				TextureStorage::push(entry.path().string());
			}
		}

		template<nv::InputEvent_C... Events>
		void pushInputEvents(Events&... events) {
			((m_inputEvents.push_back(events)), ...);
		}

		template<nv::KeyboardEvent_C... KeyEvents>
		void pushKeyboardEvents(KeyEvents&... events) {
			((m_keyboardEvents.push_back(events)), ...);
		}

		template<nv::Event_C... Events>
		void pushCustomEvents(Events&... events) {
			((m_customEvents.push_back(events)), ...);
		}
		
		void execute() {
			EventHandler::reset([&]() { quit(); }); //reinitialize the event handler

			customInit();

			for (auto& evt : m_inputEvents) { //push events 
				evt.push();
			}
			for (auto& evt : m_keyboardEvents) {
				evt.push();
			}
			for (auto& evt : m_customEvents) {
				evt.push();
			}

			Uint32 waitTime, endTime;

			while (true) {
				EventHandler::runEvents();

				waitTime = 1000 / NV_FPS;
				endTime = SDL_GetTicks() + waitTime;

				SDL_RenderClear(m_renderer);

				customRender(); 

				//checks frames, render
				if (SDL_GetTicks() < endTime) {
					SDL_Delay(endTime - SDL_GetTicks());
					SDL_RenderPresent(m_renderer);
				} else {
					SDL_RenderPresent(m_renderer);
				}
			}
		}
	};
}
