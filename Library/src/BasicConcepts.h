#pragma once

#include <concepts>
#include <ranges>
#include <vector>

#include <boost/container/flat_map.hpp>

#include <plf_hive.h>

#include <nlohmann/json.hpp>

#include <SDL3/SDL_rect.h> //SDL_FPoint
#include <SDL3/SDL_render.h>

#include "Point.h"

namespace nv {
	using nlohmann::json;
	namespace ranges = std::ranges;
	namespace chrono = std::chrono;
	namespace views = std::views;
	namespace boost_con = boost::container;

	namespace concepts {
		template<typename Object>
		concept RenderableObject = requires(Object & obj) {
			obj.render(std::declval<SDL_Renderer*>());
		};

		template<typename Object>
		concept RotateableObject = requires(Object obj) {
			{ obj } -> RenderableObject;
			obj.rotate(0.0, Point{ 0.0f, 0.0f });
			obj.setRotationCenter();
		};

		template<typename Object>
		concept ScaleableObject = requires(Object obj) {
			obj.screenScale(0.0);
			{ obj.getScreenScale() } -> std::same_as<float>;
			obj.worldScale(0.0);
			{ obj.getWorldScale() } -> std::same_as<float>;
		};

		template<typename Object>
		concept MoveableObject = requires(Object obj) {
			{ obj.move(Point{}) } -> std::same_as<void>;
			{ obj.screenMove(Point{}) } -> std::same_as<void>;
			{ obj.worldMove(Point{}) } -> std::same_as<void>;
			{ obj.setScreenPos(Point{}) } -> std::same_as<void>;
			{ obj.setWorldPos(Point{}) } -> std::same_as<void>;
			{ obj.getScreenPos() } -> std::same_as<Point>;
			{ obj.getWorldPos() } -> std::same_as<Point>;
		};

		template<typename Object>
		concept SizeableObject = requires(Object obj) {
			obj.setSize(500, 500);
			obj.setSize(Point{ 500, 500 });
			{ obj.getSize() } -> std::same_as<Point>;
		};

		template<typename Range>
		concept RenderObjectRange = ranges::viewable_range<Range> && RenderableObject<typename Range::value_type>;

		template<typename T>
		concept Aggregate = std::is_aggregate_v<std::remove_cvref_t<T>>;

		template<typename T, typename U>
		concept SameAsDecayed = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

		template<ranges::viewable_range T>
		using Subrange = decltype(ranges::subrange(std::begin(std::declval<T>()), std::end(std::declval<T>())));

		template<typename T>
		using Layers = boost_con::flat_map<int, plf::hive<T>>;

		template<typename Range>
		using ValueType = typename std::remove_cvref_t<Range>::value_type;

		using nlohmann::json;

		template<typename T>
		concept Map = requires(std::remove_cvref_t<T> t) {
			std::cmp_less(std::declval<typename T::key_type>(), std::declval<typename T::key_type>());
			t.emplace(std::declval<typename T::key_type>(), std::declval<typename T::value_type>());
		};

		template<typename T>
		concept Container = requires(T t) {
			t.begin();
			t.end();
			t.cbegin();
			t.cend();
			typename std::remove_cvref_t<T>::value_type;
			typename std::remove_cvref_t<T>::iterator;
			typename std::remove_cvref_t<T>::const_iterator;
			typename std::remove_cvref_t<T>::difference_type;
			typename std::remove_cvref_t<T>::size_type;
		};

		template<typename Range>
		concept NonContainerRange = ranges::viewable_range<Range> && !Container<Range>;

		template<typename Object>
		concept JsonSerializable = requires(Object obj) {
			{ nlohmann::adl_serializer<Object>::from_json(std::declval<json>()) } -> std::same_as<Object>;
			nlohmann::adl_serializer<Object>::to_json(std::declval<json>(), obj);
		};

		template<typename T>
		concept Primitive = std::is_fundamental_v<T> || std::is_pointer_v<T> || std::is_enum_v<T>;

		template<typename T>
		concept String = std::ranges::viewable_range<T> && std::same_as<typename T::value_type, char>;
		
		template<typename T>
		concept Printable = requires(T t) {
			std::println("{}", t);
		};
	}
}