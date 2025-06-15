#pragma once

#include <string>
#include <boost/unordered/unordered_flat_set.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

namespace nv {
	namespace editor {
		class NameManager {
		private:
			boost::unordered_flat_set<std::string> m_takenNames;
			std::string m_unnamedObjectPrefix;
			int m_uniqueTextNum = 0;
			bool m_wasJustInputtingName = false;
		public:
			NameManager(const char* unnamedObjectPrefix) :
				m_unnamedObjectPrefix{ unnamedObjectPrefix }
			{
			}

			void makeNewName(std::string& newName) {
				if (m_takenNames.contains(newName) || newName.empty()) {
					newName += ("(ID=" + std::to_string(m_uniqueTextNum) + ")");
					m_uniqueTextNum++;
				}
				if (newName == "(ID=0)") {
					static int x = 0;
					x++;
					assert(x != 2);
				}
				m_takenNames.insert(newName);
			}

			void inputName(const char* inputLabel, std::string& name) {
				using namespace std::literals;

				auto oldName = name;
				if (ImGui::InputText(inputLabel, &name)) {
					m_wasJustInputtingName = true;
					if (m_takenNames.contains(oldName)) {
						m_takenNames.erase(oldName);
					}
				}
				
				if (!ImGui::IsItemActive() && m_wasJustInputtingName) {
					if (name.empty()) {
						name = m_unnamedObjectPrefix + "(ID = "s + std::to_string(m_uniqueTextNum++) + ")";
					} else if (m_takenNames.contains(name)) {
						name += (" (Copy ID=" + std::to_string(m_uniqueTextNum++) + ")");
					}
					m_takenNames.insert(name);
					m_wasJustInputtingName = false;
				} 
			}
		};
	}
}