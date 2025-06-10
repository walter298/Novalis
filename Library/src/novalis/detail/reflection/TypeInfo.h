#pragma once

namespace nv {
	namespace detail {
		template<typename T>
		constexpr auto getTypeName() {
			static constexpr std::string_view name = __FUNCSIG__;
			if constexpr (name.contains("struct")) {
				constexpr auto start = name.find("struct") + 7;
				constexpr auto end = name.find_last_of(">");
				constexpr auto length = end - start;
				return name.substr(start, length);
			} else if constexpr (name.contains("class")) {
				constexpr auto start = name.find("class") + 6;
				constexpr auto end = name.find_last_of(">");
				constexpr auto length = end - start;
				return name.substr(start, length);
			} else if constexpr (name.contains("enum")) {
				constexpr auto start = name.find("enum") + 5;
				constexpr auto end = name.find_last_of(">");
				constexpr auto length = end - start;
				return name.substr(start, length);
			} else if constexpr (name.contains("union")) {
				constexpr auto start = name.find("union") + 6;
				constexpr auto end = name.find_last_of(">");
				constexpr auto length = end - start;
				return name.substr(start, length);
			} else if constexpr (name.contains("auto")) {
				constexpr auto start = name.find("auto") + 5;
				constexpr auto end = name.find_last_of(">");
				constexpr auto length = end - start;
				return name.substr(start, length);
			} else if constexpr (name.contains("typename")) {
				constexpr auto start = name.find("typename") + 9;
				constexpr auto end = name.find_last_of(">");
				constexpr auto length = end - start;
				return name.substr(start, length);
			} else if constexpr (name.contains("const")) {
				constexpr auto start = name.find("const") + 6;
				constexpr auto end = name.find_last_of(">");
				constexpr auto length = end - start;
				return name.substr(start, length);
			} else if constexpr (name.contains("volatile")) {
				constexpr auto start = name.find("volatile") + 9;
				constexpr auto end = name.find_last_of(">");
				constexpr auto length = end - start;
				return name.substr(start, length);
			} else if constexpr (name.contains("signed")) {
				constexpr auto start = name.find("signed") + 7;
				constexpr auto end = name.find_last_of(">");
				constexpr auto length = end - start;
				return name.substr(start, length);
			} else if constexpr (name.contains("unsigned")) {
				constexpr auto start = name.find("unsigned") + 9;
				constexpr auto end = name.find_last_of(">");
				constexpr auto length = end - start;
				return name.substr(start, length);
			}
		}

		// template<typename T>
		// struct Type {
		// 	using 
		// }
	}
}