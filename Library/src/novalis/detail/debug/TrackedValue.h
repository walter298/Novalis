#pragma once

#include <stacktrace>
#include <vector>
#include <unordered_set>
#include <print>

namespace nv {
	namespace debug {
		template<size_t... ForbiddenValues>
		class TrackedSize {
		private:
			size_t m_value;

			void printAndThrow(size_t forbiddenValue) {
				std::println(stderr, "Forbidden value encountered: {}", forbiddenValue);

				for (const auto& stacktrace : std::stacktrace::current()) {
					std::println(stderr, "{}, {}", stacktrace.source_file(), stacktrace.source_line());
					//std::println(stderr, "{}", stacktrace.description());
				}
			}
		public:
			TrackedSize(size_t t)
				: m_value{ t }
			{
				std::println("Constructing with {}", m_value);
				if (((m_value == ForbiddenValues) || ...)) {
					printAndThrow(m_value);
				}
			}

			size_t get() const noexcept {
				return m_value;
			}

			TrackedSize(const TrackedSize& other) {
				std::println("Copy constructing with {}", other.m_value);
				if (((other.m_value == ForbiddenValues) || ...)) {
					printAndThrow(other.m_value);
				}
				m_value = other.m_value;
 			}
			TrackedSize(TrackedSize&& other) {
				std::println("Move constructing with {}", other.m_value);
				if (((other.m_value == ForbiddenValues) || ...)) {
					printAndThrow(other.m_value);
				}
				m_value = std::move(other.m_value);
			}
			TrackedSize& operator=(const TrackedSize& other) {
				std::println("Copy assigning with {}", other.m_value);
				if (((other.m_value == ForbiddenValues) || ...)) {
					printAndThrow(other.m_value);
				}
				m_value = other.m_value;
				return *this;
			}
			TrackedSize& operator=(TrackedSize&& other) {
				std::println("Move assigning with {}", other.m_value);
				if (((other.m_value == ForbiddenValues) || ...)) {
					printAndThrow(other.m_value);
				}
				m_value = other.m_value;
				return *this;
			}

			bool operator==(const TrackedSize& other) const {
				return m_value == other.m_value;
			}
		};
	}
}