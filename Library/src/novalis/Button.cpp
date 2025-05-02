//#include "Button.h"
//
//nv::Button::Button(nv::Text& text, Rect& rect, Event<void>&& onClicked, Event<void>&& onHovered, Event<void>&& onUnhovered) noexcept
//	: m_text{ text }, m_rect{ rect }, m_originalTextPos{ text.ren.getPos() }, m_originalRectPos{ rect.getPos() },
//	m_onClicked{ std::move(onClicked) }, m_onHovered{ std::move(onHovered) }, m_onUnhovered{ std::move(onUnhovered) }
//{
//}
//
//void nv::Button::toggleIn() noexcept {
//	m_text.get().ren.setPos(m_originalTextPos);
//	m_rect.get().setPos(m_originalRectPos);
//}
//
//void nv::Button::toggleOut() noexcept {
//	static constexpr SDL_FPoint OUT_OF_SCENE{ 50000.0f, 50000.0f };
//	m_text.get().ren.setPos(OUT_OF_SCENE);
//	m_rect.get().setPos(OUT_OF_SCENE);
//}
//
//void nv::Button::operator()(MouseData mouseBtnData) {
//	if (m_rect.get().containsCoord({ mouseBtnData.x, mouseBtnData.y })) {
//		m_onHovered();
//		m_previouslyHovered = true;
//		if (mouseBtnData.left == MouseButtonState::Down) {
//			m_onClicked();
//		}
//	} else if (m_previouslyHovered) {
//		m_previouslyHovered = false;
//		m_onUnhovered();
//	}
//}