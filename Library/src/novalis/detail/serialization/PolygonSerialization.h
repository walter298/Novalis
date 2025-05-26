#pragma once

#include "../memory/MemoryRegion.h"
#include "../../Polygon.h"
#include "AggregateSerialization.h"

namespace nlohmann {
	struct PolygonSerializerBase {
		static constexpr const char* SCREEN_POINTS_KEY = "Screen_Points";
		static constexpr const char* WORLD_POINTS_KEY  = "World_Points";
		
		template<typename PointStorage>
		static void to_json(json& j, const nv::detail::PolygonNode<PointStorage>& p) {
			j[SCREEN_POINTS_KEY] = p.m_ren.points;
			j[WORLD_POINTS_KEY]  = p.m_world.points;
		}
	};

	template<>
	struct adl_serializer<nv::BufferedPolygon> : public PolygonSerializerBase {
		static inline nv::detail::MemoryRegion* currentPointRegion = nullptr;
		
		static nv::BufferedPolygon from_json(const json& j) {
			static size_t pointsAllocated = 0;

			assert(currentPointRegion);

			auto makePointSpan = [&](const char* key) {
				auto points = j[key].get<std::vector<nv::Point>>();
				pointsAllocated += points.size();

				auto pointsPtr = currentPointRegion->allocate<nv::Point>(points.size(), true);
				/*std::println("# of points allocated: {}", pointsAllocated);
				std::println("# of point bytes allocated: {}", pointsAllocated * sizeof(nv::Point));*/

				std::ranges::move(points, pointsPtr);
				return std::span{ pointsPtr, points.size() };
			};
			nv::detail::BufferedPolygonRep screenPoints{ makePointSpan(SCREEN_POINTS_KEY) };
			nv::detail::BufferedPolygonRep worldPoints{ makePointSpan(WORLD_POINTS_KEY) };

			return nv::BufferedPolygon{ std::move(screenPoints), std::move(worldPoints) };
		}
	};

	template<>
	struct adl_serializer<nv::DynamicPolygon> : public PolygonSerializerBase {
		static nv::DynamicPolygon from_json(const json& j) {
			//std::println("Polygon Json: \n{}", j.dump(2));
			auto screenPoints = j[SCREEN_POINTS_KEY].get<std::vector<nv::Point>>();
			auto worldPoints = j[WORLD_POINTS_KEY].get<std::vector<nv::Point>>();
			return nv::DynamicPolygon{ std::move(screenPoints), std::move(worldPoints) };
		}
	};
}