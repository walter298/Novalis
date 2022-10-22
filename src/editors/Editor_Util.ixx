#include <iostream>
#include <functional>

#include "SDL.h"


import RenderObj;

export module Editor_Util;

namespace nv {
	export std::pair<int, int> deltaMouseCoords(int& mX, int& mY) {
		int mX2, mY2;
		SDL_GetMouseState(&mX2, &mY2);

		int dX = mX2 - mX, dY = mY2 - mY;

		mX = mX2, mY = mY2; //update mouse coordinates

		return std::make_pair(dX, dY);
	}
}