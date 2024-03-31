#ifndef DATA_UTIL_H
#define DATA_UTIL_H

#include <assert.h>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/pfr.hpp>

#include <plf_hive.h>

#include <nlohmann/json.hpp>

#include <SDL2/SDL.h>

#include "Rect.h"
#include "GlobalMacros.h"
#include "Namespace.h"

void to_json(nlohmann::json& j, const SDL_Color& c);
void from_json(const nlohmann::json& j, SDL_Color& c);

void to_json(nlohmann::json& j, const SDL_Rect& c);
void from_json(const nlohmann::json& j, SDL_Rect& c);

void to_json(nlohmann::json& j, const SDL_Point& p);
void from_json(const nlohmann::json& j, SDL_Point& p);

namespace std {
	namespace chrono {
		template<typename Rep, typename Period>
		void from_json(const nlohmann::json& j, duration<Rep, Period>& time) {
			time = duration<Rep, Period>{ j.at("time").get<Rep>() };
		}
		template<typename Rep, typename Period>
		void to_json(nlohmann::json& j, const duration<Rep, Period>& time) {
			j["time"] = time.count();
		}
	}
}

namespace nv {
	template<typename T>
	concept Aggregate = std::is_aggregate_v<T>;

	namespace detail {
		template<Aggregate Aggr, size_t... Idxs>
		void assignEachAggrMember(const json& j, Aggr& aggr, std::index_sequence<Idxs...> idxs) {
			using ParsedTuple = std::tuple<typename pfr::tuple_element_t<Idxs, Aggr>...>;
			auto parsedTuple = j.get<ParsedTuple>();
			((pfr::get<Idxs>(aggr) = std::get<Idxs>(parsedTuple)), ...);
		}
	}

	template<Aggregate Aggr>
	void from_json(const json& j, Aggr& aggr) {
		detail::assignEachAggrMember(j, aggr, std::make_index_sequence<pfr::tuple_size_v<Aggr>>());
	}

	namespace detail {
		template<Aggregate Aggr, size_t... Idxs>
		auto feedJsonAggregate(json& j, const Aggr& aggr, std::index_sequence<Idxs...> idxs) {
			j = std::tie((pfr::get<Idxs>(aggr), ...));
		}
	}

	template<Aggregate Aggr>
	void to_json(json& j, const Aggr& aggr) {
		detail::feedJsonAggregate(j, aggr, std::make_index_sequence<pfr::tuple_size_v<Aggr>>());
	}

	template<std::integral... Nums>
	void parseUnderscoredNums(const std::string& line, Nums&... nums)
	{
		size_t index = 0;

		auto numify = [&](auto& x) {
			size_t iIndex = index; //initial index

			while (true) {
				index++;
				if (line[index] == '_' || index == line.size()) {
					index++;
					break;
				}
			}

			x = std::stoi(line.substr(iIndex, index));
		};

		(numify(nums), ...);
	}

	const std::string& workingDirectory();
	
	inline std::string objectPath(std::string relativePath) {
		return workingDirectory() + std::string("static_objects/") + relativePath;
	}

	inline std::string imagePath(std::string relativePath) {
		return workingDirectory() + std::string("images/") + relativePath;
	}

	//returns the path relative to the working directory
	inline std::string relativePath(std::string relativePath) {
		return workingDirectory() + relativePath;
	}

	template<typename Stream>
	Stream& operator<<(Stream& stream, const SDL_Rect& rect) {
		stream << rect.x << "_" << rect.y << "_" << rect.w << "_" << rect.h << std::endl;
		return stream;
	}

	template<typename Stream, typename... Args>
	void writeSection(Stream& stream, std::string title, Args&&... args) {
		stream << title + " {\n";
		((stream << args << '\n'), ...);
		stream << "}\n";
	}

	std::optional<std::string> fileExtension(const std::string& fileName);
	std::string fileName(const std::string& filePath);

	template<typename... Args>
	void println(Args&&... args) {
		((std::cout << std::forward<Args>(args) << " "), ...);
		std::cout << '\n';
	}

	template<typename T, typename U>
	using FlatOrderedMap = boost::container::flat_map<T, U>;

	//convenience routine for plf::hive
	template<typename T>
	decltype(auto) getBack(T& container) {
		return *(std::prev(container.end()));
	}
};

#endif