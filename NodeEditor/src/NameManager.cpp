#include "ImGuiID.h"
#include "NameManager.h"

std::string nv::editor::NameManager::makeUniqueName() {
	auto newName = (m_unnamedObjectPrefix + "#" + std::to_string(m_uniqueTextNum));
	m_uniqueTextNum++;
	m_takenNames.insert(newName);
	return newName;
}

void nv::editor::NameManager::makeExistingNameUnique(std::string& newName) {
	if (m_takenNames.contains(newName)) {
		newName += (" (Copy) (ID=" + std::to_string(m_uniqueTextNum) + ")");
		m_uniqueTextNum++;
		m_takenNames.insert(newName);
	} else if (newName.empty()) {
		newName = makeUniqueName();
	}
}

bool nv::editor::NameManager::inputName(const char* inputLabel, std::string& name) {
	using namespace std::literals;

	ImGui::PushID(getTemporaryImGuiID());
	auto oldName = name;
	if (ImGui::InputText(inputLabel, &name)) {
		m_wasJustInputtingName = true;
		if (m_takenNames.contains(oldName)) {
			m_takenNames.erase(oldName);
		}
	}
	ImGui::PopID();

	if (!ImGui::IsItemActive() && m_wasJustInputtingName) {
		makeExistingNameUnique(name);
		m_takenNames.insert(name);
		m_wasJustInputtingName = false;
		return true;
	}

	return false;
}
