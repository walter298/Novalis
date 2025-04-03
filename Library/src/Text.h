//#pragma once
//
//#include <memory>
//#include <string>
//#include <string_view>
//#include <boost/unordered/unordered_flat_map.hpp>
//
//#include <SDL3_ttf/SDL_ttf.h>
//
//#include "Rect.h"
//
//namespace nv {
//	using FontRAII = std::unique_ptr<TTF_Font, void(*)(TTF_Font*)>;
//	using FontMap  = boost::unordered_flat_map<std::string, FontRAII>;
//
//	FontRAII loadFont(std::string_view fontPath, int fontSize);
//
//	class Text {
//	private:
//		SDL_Renderer* m_renderer;
//		TTF_Font* m_font;
//		TexturePtr m_tex; 
//		std::string m_str;
//		std::string m_fontPath;
//		int m_fontSize = 0;
//		void changeText(std::string_view newText) noexcept;
//	public:
//		Rect ren;
//		SDL_Color color{ 0, 0, 0, 255 };
//
//		Text() = default;
//		Text(SDL_Renderer* renderer, std::string_view str, std::string_view fontPath, int fontSize, TTF_Font* font);
//		Text(SDL_Renderer* renderer, std::string_view str, int fontSize, TTF_Font* font);
//
//		void operator=(std::string_view str) noexcept;
//
//		std::string_view value() const noexcept;
//
//		void screenScale(float newScale) noexcept {
//			ren.scale(newScale);
//		}
//		void screenMove(SDL_FPoint change) noexcept {
//			ren.move(change);
//		}
//		SDL_FPoint getScreenPos() const noexcept {
//			return ren.getPos();
//		}
//		SDL_FPoint getScreenSize() const noexcept {
//			return ren.getSize();
//		}
//		void setScreenSize(SDL_FPoint p) noexcept {
//			ren.setSize(p);
//		}
//
//		void setOpacity(uint8_t a) noexcept {
//			color.a = a;
//		}
//
//		void render(SDL_Renderer* renderer) const noexcept {
//			SDL_RenderTexture(m_renderer, m_tex.tex, nullptr, &ren.rect);
//		}
//
//		void save(json& json) const;
//	};
//
//	using TextRef = std::reference_wrapper<Text>;
//
//	class TextInput {
//	private:
//		Rect m_rect;
//		std::reference_wrapper<Text> m_text;
//		bool m_mouseClickedAndInRegion = false;
//		std::string m_buff;
//
//		chrono::system_clock::time_point m_lastTimePopped = chrono::system_clock::now();
//		bool tooSoonToPop() const noexcept;
//	public:
//		TextInput(const Rect& rect, Text& text);
//
//		const nv::Rect& getRect() const noexcept;
//
//		void append(std::string_view inputText);
//		void pop();
//	};
//}