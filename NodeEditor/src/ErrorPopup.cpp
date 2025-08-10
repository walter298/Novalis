#include "ErrorPopup.h"
#include "WindowLayout.h"

void nv::editor::ErrorPopup::show() noexcept {
	ImGui::SetNextWindowPos(getErrorWindowPos());
	ImGui::SetNextWindowSize(getErrorWindowSize());
	ImGui::Begin(ERROR_POPUP_NAME, nullptr, DEFAULT_WINDOW_FLAGS);
	for (const auto& msg : m_errorMessages) {
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f });
		ImGui::TextWrapped(msg.c_str());
		ImGui::PopStyleColor();
	}
	ImGui::End();
}