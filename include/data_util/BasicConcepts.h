#pragma once

#include <concepts>
#include <ranges>
#include <vector>

#include <boost/container/flat_map.hpp>

#include <nlohmann/json.hpp>

#include <SDL2/SDL_rect.h> //SDL_Point

namespace nv {
	template<typename Object>
	concept RenderObject = requires(Object & obj) {
		obj.move(1, -1);
		obj.move(SDL_Point{});
		obj.scale(1, -1);
		obj.scale(SDL_Point{});
		obj.containsCoord(1, -1);
		obj.containsCoord(SDL_Point{});
		obj.render();
	};

	template<typename Object>
	concept RotatableObject = requires(Object obj) {
		{ obj } -> RenderObject;
		obj.rotate(0.0, SDL_Point{});
		obj.setRotationCenter();
	};

	template<typename Object>
	concept SizeableObject = requires(Object obj) {
		obj.setSize(500, 500);
		obj.setSize(SDL_Point{ 500, 500 });
		{ obj.getSize() } -> std::same_as<SDL_Point>;
	};

	namespace ranges    = std::ranges;
	namespace chrono    = std::chrono;
	namespace views     = std::views;
	namespace boost_con = boost::container;

	using namespace std::literals;

	template<typename Range>
	concept RenderObjectRange = ranges::viewable_range<Range> && RenderObject<typename Range::value_type>;

	template<typename... Ts>
	using ObjectLayers = boost_con::flat_map<int, std::tuple<std::vector<Ts>...>>;

	template<typename T>
	concept Aggregate = std::is_aggregate_v<std::remove_cvref_t<T>>;

	template<typename T, typename U>
	concept SameAsDecayed = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

	template<typename T>
	using Subrange = ranges::subrange<typename std::vector<T>::iterator>;

	template<typename T>
	using Layers = boost_con::flat_map<int, std::vector<T>>;

	template<typename Range>
	using ValueType = typename std::remove_cvref_t<Range>::value_type;

	using nlohmann::json;
}