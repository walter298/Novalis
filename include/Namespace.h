#pragma once

#include <chrono>
#include <ranges>

#include <boost/pfr.hpp>

#include <nlohmann/json.hpp>

namespace nv {
	//namespace aliases
	namespace time = std::chrono;
	namespace ranges = std::ranges;
	namespace views = std::views;
	namespace pfr = boost::pfr;

	//class aliases
	using nlohmann::json;

	//literals
	using namespace std::literals;
}