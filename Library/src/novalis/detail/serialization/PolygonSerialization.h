#pragma once

#include "../memory/MemoryRegion.h"
#include "../../Polygon.h"
#include "AutoSerialization.h"

namespace nlohmann {
	struct PolygonSerializerBase {
		static constexpr const char* POINTS_KEY = "Screen_Points";
		static constexpr const char* ANGLE_KEY = "Angle";
		static constexpr const char* SCALE_KEY = "Scale";

		template<typename PointStorage>
		static void to_json(json& j, const nv::detail::Polygon<PointStorage>& p) {
			j[POINTS_KEY] = p.points;
			j[ANGLE_KEY] = p.currAngle;
			j[SCALE_KEY] = p.currScale;
		}
	};

	template<>
	struct adl_serializer<nv::detail::Polygon<std::vector<nv::Point>>> : public PolygonSerializerBase
	{
		static nv::detail::Polygon<std::vector<nv::Point>> from_json(const json& j) {
			auto points = j[POINTS_KEY].get<std::vector<nv::Point>>();
			float angle = j[ANGLE_KEY].get<float>();
			float scale = j[SCALE_KEY].get<float>();
			return nv::detail::Polygon<std::vector<nv::Point>>{ std::move(points), angle, scale };
		}
	};

	template<>
	struct adl_serializer<nv::detail::Polygon<std::span<nv::Point>>> : public PolygonSerializerBase
	{
		static inline nv::detail::MemoryRegion* currentPointRegion = nullptr;

		static nv::detail::Polygon<std::span<nv::Point>> from_json(const json& j) {
			auto points = j[POINTS_KEY].get<std::vector<nv::Point>>();
			auto pointsPtr = currentPointRegion->allocate<nv::Point>(points.size());
			std::ranges::move(points, pointsPtr);

			float angle = j[ANGLE_KEY].get<float>();
			float scale = j[SCALE_KEY].get<float>();
			return nv::detail::Polygon<std::span<nv::Point>>{ 
				std::span{ pointsPtr, points.size() }, angle, scale 
			};
		}
	};

	template<typename PointStorage>
	struct adl_serializer<nv::detail::PolygonNode<PointStorage>> {
		static constexpr const char* REN_KEY   = "Ren";
		static constexpr const char* WORLD_KEY = "World";

		static nv::detail::PolygonNode<PointStorage> from_json(const json& j) {
			auto ren   = j[REN_KEY].get<nv::detail::Polygon<PointStorage>>();
			auto world = j[WORLD_KEY].get<nv::detail::Polygon<PointStorage>>();
			return nv::detail::PolygonNode<PointStorage>{ std::move(ren), std::move(world) };
		}
		static void to_json(json& j, const nv::detail::PolygonNode<PointStorage>& p) {
			j[REN_KEY]   = p.m_ren;
			j[WORLD_KEY] = p.m_world;
		}
	};
}