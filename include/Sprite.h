#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include <fstream>
#include <filesystem>
#include <iostream>
#include <memory>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#undef min
#undef max

#include <plf_hive.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "DataUtil.h"
#include "ID.h"
#include "Namespace.h"
#include "Texture.h"

namespace nv {
	//enum class FontType {
	//	WorkSans,
	//	Libertine,
	//};

	//using FontMap = std::unordered_map<FontType, TTF_Font*>;

	//namespace editor {
	//	class TextEditor;
	//}

	//class Text : public RenderObj {
	//private:
	//	TexturePtr m_tex = std::make_shared<Texture>(nullptr);
	//	Rect m_rect;
	//	TTF_Font* m_font = nullptr;
	//	std::string m_text;
	//	SDL_Color m_color{ 255, 255, 255, 0 };
	//	int m_fontSize = 14;

	//	nv::Rect m_background{ 0, 0, 0, 0 };
	//	nv::Rect m_ren{ 0, 0, 0, 0 };

	//	static inline FontMap fontMap;
	//public:
	//	//these methods should ONLY be called by nv::Instance
	//	static void openFonts() noexcept;
	//	static void closeFonts() noexcept;

	//	Text() = default; 
	//	Text(SDL_Renderer* renderer, std::string absPath);

	//	virtual void render(SDL_Renderer* renderer) noexcept override;

	//	void setRenPos(int x, int y) noexcept;
	//	void setFontSize(SDL_Renderer* renderer, int fontSize) noexcept;
	//	void changeText(SDL_Renderer* renderer, std::string text) noexcept;
	//	void setColor(SDL_Color color) noexcept;
	//	
	//	friend class editor::TextEditor;
	//};

	enum class Flip {
		None,
		Horizontal,
		Vertical
	};

	//friend classes
	namespace editor {
		class SceneEditor; //friend of Sprite and Background
		class SpriteEditor; //friend of Sprite
		class BackgroundEditor; //friend of Background
	}

	namespace detail {
		using Textures = std::shared_ptr<std::vector<Texture>>; //alias is used by sprite and background
		void loadTextures(json& j, SDL_Renderer* renderer, Textures& textures);
	}

	class Sprite : public IDObj {
	private:
		detail::Textures m_textures{ nullptr };
		std::string m_name;
		size_t m_currTexGroupIdx = 0;
		TexturePos m_pos;
	public:
		Sprite() = default;
		Sprite(SDL_Renderer* renderer, const std::string& path, const std::string& name = "");
		
		Rect ren;
		Rect world;

		const std::string& getName() const noexcept;
		
		void changeTexture(size_t texIdx) noexcept;
		void flip(SDL_RendererFlip flip);
		void rotate(double angle, int x, int y) noexcept;
		void render(SDL_Renderer* renderer) const noexcept;
		void renMove(int dx, int dy) noexcept;

		friend class editor::SceneEditor;
		friend class editor::SpriteEditor;
	};

	using Sprites = plf::hive<Sprite>;

	class Background;

	class Background : public IDObj {
	private:
		detail::Textures m_textures;
		int m_x = 0;
		int m_y = 0;
		int m_width = 0;
		int m_height = 0;

		int m_horizTexC = 1;
	public:
		nv::Rect ren{ 0, 0, 0, 0 };

		Background() = default;
		Background(SDL_Renderer* renderer, const std::string& path);
		void render(SDL_Renderer* renderer) const noexcept;

		friend class editor::BackgroundEditor;
		friend class editor::SceneEditor;
	};

	/*using TextPtr = std::unique_ptr<Text>;

	template<typename Stream> 
	Stream& operator<<(Stream& stream, const Text& text) {
		json j;
		j["type"]        = "text"s;
		j["name"]        = text.m_name;
		j["text"]        = text.m_text;
		j["color"]       = text.m_color;
		j["background"]  = text.m_background;
		j["ren"]         = text.m_ren;
		stream << j.dump(2);
		return stream;
	}*/
}

#endif