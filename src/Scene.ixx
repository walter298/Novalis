#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "SDL.h"
#include "SDL_image.h"

export module Scene;

import RenderObj;
import Data_Util;
import Game;
import EventHandler;

namespace nv {
	export class Scene : public Game {
	private:
		struct Background : public RenderObj {
			Background(std::string path, int x, int y) {
				m_currentSprite = IMG_LoadTexture(m_renderer, path.c_str());
				m_ren = { x * NV_SCREEN_WIDTH, y * NV_SCREEN_HEIGHT, NV_SCREEN_WIDTH, NV_SCREEN_HEIGHT };
			}
		};

		typedef std::shared_ptr<RenderObj> RenderObjPtr;

		std::vector<RenderObjPtr> m_owningObjectPtrs, m_backgrounds, m_objects, m_cameraFreeObjects;

		virtual void customRender() override { //copy objects to renderer
			//std::cout << "rendering\n";
			for (auto& background : m_backgrounds) {
				//std::cout << "rendering backgrounds\n";
				SDL_RenderCopy(m_renderer, background.get()->getSprite(), NULL, background.get()->getRen());
			}
			for (auto& obj : m_objects) {
				SDL_RenderCopy(m_renderer, obj.get()->getSprite(), NULL, obj.get()->getRen());
			}
		}
	public:
		Scene(std::string path) {
			std::string fullPath = NV_WORKING_DIRECTORY + path;

			std::ifstream sceneFile;
			sceneFile.open(fullPath);

			if (!sceneFile.is_open()) {
				std::cout << "Error: the file " << fullPath << " does not exist\n";
				exit(1);
			}

			std::string section;
			std::vector<std::string> sectionData;

			std::string line;

			while (std::getline(sceneFile, line)) {
				if (line.empty()) { //ignore empty lines
					continue;
				}

				if (line.back() == '{') {
					section = line;
					continue;
				}

				if (line == "}") {
					if (section == "STATIC_TEXTURES {") { //load in objects and set their positions
						for (std::string& str : sectionData) {
							auto [path, wx, wy] = staticObjectData(str);

							auto obj = std::make_shared<RenderObj>(TextureStorage::get(objectPath(path)));
							obj.get()->setPos(wx, wy); 

							m_owningObjectPtrs.push_back(obj);
							m_objects.push_back(obj);
						}
					} else if (section == "BACKGROUNDS {") { //load background images and set their positions
						for (std::string& str : sectionData) {
							auto [path, x, y] = staticObjectData(str);

							std::shared_ptr<RenderObj> background =
								std::make_shared<Background>(Background(imagePath(path), x, y));
							
							m_owningObjectPtrs.push_back(background);
							m_backgrounds.push_back(background);

							//crash end point
						}
					}
					sectionData.clear();
				} else { //add data if still within section
					sectionData.push_back(line);
				}
			}

			m_cameraFreeObjects = m_owningObjectPtrs; //set camera free objects
		}

		void camMove(int dx, int dy) {
			for (auto& obj : m_cameraFreeObjects) {
				obj.get()->renMove(dx, dy);
			}
		}

		Scene() = default;
	};
}