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

			std::string makeUniqueName();
			void makeNewName(std::string& newName);
			void inputName(const char* inputLabel, std::string& name);

			void deleteName(const std::string& name) {
				m_takenNames.erase(name);
			}
		};
	}
}