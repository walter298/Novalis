#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>

#include "SDL.h"
#include "SDL_image.h"

import Data_Util;

export module RenderObj;

namespace nv {
	export typedef std::vector<SDL_Texture*> TexVec;

	export class RenderObj {
	private:
		std::string m_path;
		std::vector<std::string> m_spriteImagePaths;
	protected:
		TexVec m_spriteSheet;

		size_t m_spriteIndex = 0;

		SDL_Texture* m_currentSprite;

		SDL_Rect m_ren = { 0, 0, 500, 500 };
		SDL_Rect m_world = { 0, 0, 500, 500 };
	public:
		RenderObj(SDL_Renderer* renderer, std::string path) 
			: m_path(path) 
		{
			std::ifstream file;
			file.open(path);

			if (!file.is_open()) {
				std::cout << "Error: renderObj file is not open\n";
				std::cout << "Path: " << path << std::endl;
			}

			std::string section;
			std::vector<std::string> sectionData; //can be texture path list, rect, or collision box

			std::string line;

			while (std::getline(file, line)) {
				if (line.empty()) { //ignore empty lines
					continue;
				}

				if (line.back() == '{') { //update parsed section
					section = line;
					continue;
				}

				//if start of new section, parse then clear the current section
				if (line == "}") {
					if (section == "SPRITE_SHEET {") { //load in textures
						for (auto& line : sectionData) {
							m_spriteImagePaths.push_back(line); //retain path data
							line.insert(0, NV_WORKING_DIRECTORY);
							m_spriteSheet.push_back((IMG_LoadTexture(renderer, line.c_str())));
						}
					} else if (section == "RENDER {") { //set texture width and height
						int x, y, w, h;
						parseSpacedNums(sectionData[0], x, y, w, h);
						m_ren = { x, y, w, h };
					} else if (section == "COLLISION {") {
						int x, y, w, h;
						parseSpacedNums(sectionData[0], x, y, w, h);
						m_world = { x, y, w, h };
					}
					sectionData.clear();
				} else {
					sectionData.push_back(line);
				}
			}
			
			m_currentSprite = m_spriteSheet[0];
		}

		void writeData() {
			std::ofstream file;
			file.open(m_path, std::ofstream::trunc); 

			//write sprite sheet data
			file << "SPRITE_SHEET {\n";
			for (std::string& path : m_spriteImagePaths) {
				file << path << std::endl;
			}
			file << "}\n";

			//write render data
			file << "SCREEN_RES {\n";
			file << writeNums(m_ren.x, m_ren.y, m_ren.w, m_ren.h) << std::endl;
			file << "}\n";

			//write collision data
			file << "COLLISION {\n";
			file << writeNums(m_world.x, m_world.y, m_world.w, m_world.h) << std::endl;
			file << "}\n";

			file.close();
		}

		RenderObj() = default;

		int rX() {
			return m_ren.x;
		}

		int rY() {
			return m_ren.y;
		}

		int rW() {
			return m_ren.w;
		}

		int rH() {
			return m_ren.h;
		}

		SDL_Texture* getSprite() {
			return m_currentSprite;
		}

		SDL_Rect* getRen() {
			return &m_ren;
		}

		SDL_Rect* getWorld() {
			return &m_world;
		}

		void renMove(int dx, int dy) {
			m_ren.x += dx;
			m_ren.y += dy;
		}

		void worldMove(int dx, int dy) {
			m_world.x += dx;
			m_world.y += dy;
		}

		void move(int dx, int dy) {
			renMove(dx, dy);
			worldMove(dx, dy);
		}

		void setRenPos(int x, int y) {
			m_ren.x = x;
			m_ren.y = y;
		}

		void setWorldPos(int x, int y) {
			m_world.x = x;
			m_world.y = y;
		}

		void setPos(int x, int y) {
			int dX, dY; //relative distance from render x and y to world x and y
			dX = m_ren.x - m_world.x;
			dY = m_ren.y - m_world.y;

			setWorldPos(x, y);
			setRenPos(x + dX, y + dY);
		}

		void renScale(int dw, int dh) {
			m_ren.w += dw;
			m_ren.h += dh;
		}

		void worldScale(int dw, int dh) {
			m_world.w += dw;
			m_world.h += dh;
		}

		void scale(int dw, int dh) {
			renScale(dw, dh);
			worldScale(dw, dh);
		}

		bool mouseHovered(const int& mX, const int& mY) {
			return (mX > m_ren.x && mX < m_ren.x + m_ren.w &&
				mY > m_ren.y && mY < m_ren.y + m_ren.h);
		}
	};

	export std::string objectPath(std::string relativePath) {
		return NV_WORKING_DIRECTORY + std::string("static_objects/") + relativePath;
	}

	export std::string imagePath(std::string relativePath) {
		return NV_WORKING_DIRECTORY + std::string("images/") + relativePath;
	}

	export std::string workingPath(std::string relativePath) {
		return NV_WORKING_DIRECTORY + std::string(relativePath);
	}
}