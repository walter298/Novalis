#pragma once

#include <string>
#include <vector>

namespace nv {
	namespace editor {
		class ErrorPopup {
		private:
			std::vector<std::string> m_errorMessages;
		public:
			void show() noexcept;

			template<typename T>
			void add(T&& msg) requires(std::constructible_from<std::string, T>) {
				m_errorMessages.emplace_back(std::forward<T>(msg));
			}
		};
	}
}