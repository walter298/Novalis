#include "NameManager.h"

std::string nv::editor::NameManager::makeUniqueName() {
	auto newName = (m_unnamedObjectPrefix + "#" + std::to_string(m_uniqueTextNum));
	m_uniqueTextNum++;
	m_takenNames.insert(newName);
	return newName;
}

void nv::editor::NameManager::makeNewName(std::string& newName) {
	if (m_takenNames.contains(newName)) {
		newName += (" (Copy) (ID=" + std::to_string(m_uniqueTextNum) + ")");
		m_uniqueTextNum++;
		m_takenNames.insert(newName);
	} else if (newName.empty()) {
		newName = makeUniqueName();
	}
}

void nv::editor::NameManager::inputName(const char* inputLabel, std::string& name) {
	using namespace std::literals;

	auto oldName = name;
	if (ImGui::InputText(inputLabel, &name)) {
		m_wasJustInputtingName = true;
		if (m_takenNames.contains(oldName)) {
			m_takenNames.erase(oldName);
		}
	}

	if (!ImGui::IsItemActive() && m_wasJustInputtingName) {
		makeNewName(name);
		m_takenNames.insert(name);
		m_wasJustInputtingName = false;
	}
}
