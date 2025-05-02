//#include "Text.h"
//
//#include "data_util/DataStructures.h"
//#include "data_util/File.h"
//
//#include <print>
//
//nv::FontRAII nv::loadFont(std::string_view fontPath, int fontSize) {
//	return FontRAII{ TTF_OpenFont(fontPath.data(), fontSize), TTF_CloseFont };
//}
//
//void nv::Text::changeText(std::string_view newText) noexcept {
//	m_str = newText;
//
//	auto surface = TTF_RenderText_Solid(m_font, m_str.c_str(), m_str.size(), { 0, 0, 0, 255 });
//	ScopeExit freeSurface{ [&] { SDL_DestroySurface(surface); } };
//
//	if (surface == nullptr) {
//		ren.setSize(0, 0);
//		return;
//	}
//	auto w = surface->w;
//	auto h = surface->h;
//
//	m_tex.tex = SDL_CreateTextureFromSurface(m_renderer, surface);
//	ren.setSize(static_cast<float>(w), static_cast<float>(h));
//}
//
//nv::Text::Text(SDL_Renderer* renderer, std::string_view str, std::string_view fontPath, int fontSize, TTF_Font* font)
//	: m_renderer{ renderer }, m_font{ font }, m_str{ str }, m_fontPath{ fontPath }, m_fontSize{ fontSize }
//{
//	changeText(str);
//}
//
//nv::Text::Text(SDL_Renderer* renderer, std::string_view str, int fontSize, TTF_Font* font)
//	: Text(renderer, str, "", fontSize, font)
//{
//}
//
////nv::Text::Text(SDL_Renderer* renderer, const json& json, FontMap& fontMap) 
////	: m_renderer{ renderer }
////{
////	m_fontPath = relativePath(json["font_path"].get<std::string>());
////	m_fontSize = json["font_size"].get<int>();
////
////	auto fontName = m_fontPath + std::to_string(m_fontSize);
////	auto fontPathIt = fontMap.find(fontName);
////	if (fontPathIt == fontMap.end()) {
////		auto font = loadFont(m_fontPath, m_fontSize);
////		m_font = font.get();
////		fontMap.emplace(fontName, std::move(font));
////	} else {
////		m_font = fontPathIt->second.get();
////	}
////	m_str = json["value"].get<std::string>();
////	changeText(m_str);
////	ren = json["ren"].get<Rect>();
////}
//
//void nv::Text::operator=(std::string_view str) noexcept {
//	changeText(str);
//}
//
//std::string_view nv::Text::value() const noexcept {
//	return m_str;
//}
//
////void nv::Text::setOpacity(uint8_t a) noexcept {
////	color.a = a;
////}
//
//void nv::Text::save(json& json) const {
//	json["value"]     = m_str;
//	json["font_path"] = m_fontPath;
//	json["font_size"] = m_fontSize;
//	json["ren"]       = ren;
//}
//
//bool nv::TextInput::tooSoonToPop() const noexcept {
//	static constexpr auto MAX_EDIT_WAIT_TIME = 150ms;
//	return chrono::system_clock::now() - m_lastTimePopped < MAX_EDIT_WAIT_TIME;
//}
//
//nv::TextInput::TextInput(const Rect& rect, Text& text)
//	: m_rect{ rect }, m_text{ text }
//{
//	m_buff.reserve(100);
//}
//
//const nv::Rect& nv::TextInput::getRect() const noexcept {
//	return m_rect;
//}
//
//void nv::TextInput::append(std::string_view inputText) {
//	m_buff.append(inputText);
//	m_text.get() = m_buff;
//}
//
//void nv::TextInput::pop() {
//	if (tooSoonToPop()) {
//		return;
//	}
//	auto textValue = m_text.get().value();
//	if (!textValue.empty()) {
//		m_text.get() = textValue.substr(0, textValue.size() - 1);
//		m_buff.pop_back();
//	}
//	m_lastTimePopped = chrono::system_clock::now();
//}