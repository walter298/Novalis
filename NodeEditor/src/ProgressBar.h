#include <novalis/Rect.h>

namespace nv {
	namespace editor {
		inline void showProgress(SDL_Renderer* renderer, int stepsTaken, int totalSteps) {
			auto winDimensions = getScreenDimensions();
			static constexpr float WIDTH = 250.0f;
			static constexpr float HEIGHT = 100.0f;
			SDL_FRect fullBar{
				(winDimensions.w / 2.0f) - (WIDTH / 2.0f),
				(winDimensions.h / 2.0f) - (HEIGHT / 2.0f)
			};
			auto progressBar = fullBar;
			progressBar.w = fullBar.w * static_cast<float>(stepsTaken) / static_cast<float>(stepsTaken);

			renderSDLRect(renderer, fullBar, { 255, 255, 255, 255 });
			renderSDLRect(renderer, progressBar, { 0, 255, 0, 255 });
		}
	}
}