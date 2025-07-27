#include "SpritesheetDimensionInput.h"
#include "WindowLayout.h"

static void inputSpritesheetNum(const char* label, int& num) noexcept {
	auto temp = num;
	ImGui::SetNextItemWidth(nv::editor::getInputWidth());
	ImGui::InputInt(label, &temp);
	if (temp > 0 && temp < 100) {
		num = temp;
	}
}

void nv::editor::inputSpritesheetDimensions(int& rowC, int& colC) {
	inputSpritesheetNum("rows", rowC);
	inputSpritesheetNum("columns", colC);
}
