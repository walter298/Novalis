#pragma once

#include <string>
#include <unordered_set>
#include <boost/unordered/unordered_flat_set.hpp>
#include <novalis/detail/reflection/ClassIteration.h>

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

namespace nv {
	namespace editor {
		class NameManager {
		private:
			std::unordered_set<std::string> m_takenNames;
			std::string m_unnamedObjectPrefix;
			int m_uniqueTextNum = 0;
			bool m_wasJustInputtingName = false;
		public:
			NameManager() = default;
			NameManager(const char* unnamedObjectPrefix) :
				m_unnamedObjectPrefix{ unnamedObjectPrefix }
			{
			}

			std::string makeUniqueName();
			void makeExistingNameUnique(std::string& newName);
			bool inputName(const char* inputLabel, std::string& name); //returns whether we are finished inputting name

			void deleteName(const std::string& name) {
				m_takenNames.erase(name);
			}

			MAKE_INTROSPECTION(m_takenNames, m_unnamedObjectPrefix, m_uniqueTextNum, m_wasJustInputtingName)
		};
	}
}