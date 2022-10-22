#include <iostream>

#include "SDL.h"

#undef main

import Game;
import Scene;
import SceneEditor;
import RenderObjEditor;
import Data_Util;

int main() {
	nv::SceneEditor editor("scenes/level_1.txt");
	
	editor.execute();
}