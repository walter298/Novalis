#include <SDL.h>
#include <string>
#include <iostream>

import Game;
import Scene;
import RenderObj;

export module SceneEditor;

//foo 

namespace nv {
	export class SceneEditor : public Scene {
	private:
		KeyboardEvent moveCamLeft = KeyboardEvent(
			SDL_SCANCODE_LEFT, 
			[&]() {
				camMove(20, 0);
			}
		);
		KeyboardEvent moveCamRight = KeyboardEvent(
			SDL_SCANCODE_RIGHT,
			[&]() {
				camMove(-20, 0);
			}
		);
		KeyboardEvent moveCamUp = KeyboardEvent(
			SDL_SCANCODE_UP,
			[&]() {
				camMove(0, -20);
			}
		);
		KeyboardEvent moveCamDown = KeyboardEvent(
			SDL_SCANCODE_DOWN,
			[&]() {
				camMove(0, 20);
			}
		);

		virtual void customInit() override {
			pushKeyboardEvents(moveCamLeft, moveCamRight, moveCamUp, moveCamDown);
		}
	public:
		SceneEditor(std::string path) 
			: Scene(path) 
		{} 
	};
}