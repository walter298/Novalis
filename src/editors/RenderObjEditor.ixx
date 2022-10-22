#include <string>
#include <iostream>
#include <functional>
#include <array>
#include <map>

#include "SDL_events.h"
#include "SDL_render.h"

import Game;
import RenderObj;
import EventHandler;
import Editor_Util;

export module RenderObjEditor;

namespace nv {
	export class RenderObjEditor : public Game {
	private:
		RenderObj m_obj, m_legend;

		std::string m_filePath;

		bool m_mouseClicked = false;

		enum class ToggleState {
			DRAG,
			REN_SCALE,
			WORLD_SCALE,
			WORLD_DRAG
		};

		ToggleState m_toggleStateArr[4] = {
			ToggleState::DRAG,
			ToggleState::REN_SCALE,
			ToggleState::WORLD_SCALE,
			ToggleState::WORLD_DRAG
		};

		size_t m_toggleStateIndex = 0;

		ToggleState& m_toggleState = m_toggleStateArr[m_toggleStateIndex];
	public:
		RenderObjEditor(std::string path) {
			m_obj = TextureStorage::get(path); //load in the renderObj
			m_legend = TextureStorage::get(nv::objectPath("texture_legend_obj.txt"));

			m_filePath = path;
		}

		virtual void customRender() override {
			switch (m_toggleStateArr[m_toggleStateIndex]) {
			case ToggleState::DRAG:
				break;
			case ToggleState::REN_SCALE:
				SDL_SetRenderDrawColor(m_renderer, 240, 0, 0, 0);
				SDL_RenderFillRect(m_renderer, m_obj.getRen());
				break;
			case ToggleState::WORLD_DRAG:
				SDL_SetRenderDrawColor(m_renderer, 255, 100, 0, 0);
				SDL_RenderFillRect(m_renderer, m_obj.getWorld());
				break;
			case ToggleState::WORLD_SCALE:
				SDL_SetRenderDrawColor(m_renderer, 240, 33, 255, 0);
				SDL_RenderFillRect(m_renderer, m_obj.getWorld());
				break;
			}

			SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
			SDL_RenderCopy(m_renderer, m_legend.getSprite(), NULL, m_legend.getRen());
			SDL_RenderCopy(m_renderer, m_obj.getSprite(), NULL, m_obj.getRen());
		}

		virtual void customInit() override {
			m_legend.setPos(1000, 800);
			
			nv::InputEvent mouseDown(SDL_MOUSEBUTTONDOWN,
				[&]() {
					m_mouseClicked = true;
					SDL_GetMouseState(&m_mouseX, &m_mouseY);
				}
			);
			nv::InputEvent mouseUp(SDL_MOUSEBUTTONUP,
				[&]() { 
					m_mouseClicked = false; 
					SDL_GetMouseState(&m_mouseX, &m_mouseY); 
				}
			);
			nv::InputEvent mouseMove(SDL_MOUSEMOTION,
				[&]() { 
					if (m_mouseClicked) {
						auto [dMX, dMY] = deltaMouseCoords(m_mouseX, m_mouseY);

						switch (m_toggleStateArr[m_toggleStateIndex]) {
						case ToggleState::DRAG:
							m_obj.move(dMX, dMY);
							break;
						case ToggleState::REN_SCALE:
							m_obj.renScale(dMX, dMY);
							break;
						case ToggleState::WORLD_SCALE:
							m_obj.worldScale(dMX, dMY);
							break;
						case ToggleState::WORLD_DRAG:
							m_obj.worldMove(dMX, dMY);
							break;
						}
					}
				}
			);
			nv::InputEvent mouseScroll(SDL_MOUSEWHEEL,
				[&]() {
					if (EventHandler::getEvent().wheel.y > 0) {
						if (m_toggleStateIndex == 3) {
							m_toggleStateIndex = 0;
						} else {
							m_toggleStateIndex++;
						}
					} else if (EventHandler::getEvent().wheel.y < 0) {
						if (m_toggleStateIndex == 0) {
							m_toggleStateIndex = 3;
						} else {
							m_toggleStateIndex--;
						}
					}
				}
			);
			nv::KeyboardEvent escape(SDL_SCANCODE_ESCAPE,
				[&]() {
					m_obj.writeData();

					quit();
				}
			);
			
			Game::pushInputEvents(mouseDown, mouseUp, mouseMove, mouseScroll);
			Game::pushKeyboardEvents(escape);
		}
	};
}