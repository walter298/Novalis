#pragma once

#include <string>
#include <magic_enum/magic_enum.hpp>

namespace nv {
	namespace editor {
		template<typename Enum>
		void getEnumName(std::string& buff, Enum value) {
			auto enumName = magic_enum::enum_name(value);
			buff.assign(enumName.data(), enumName.size());
		}
	}
}