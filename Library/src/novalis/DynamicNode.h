#pragma once

#include <deque>

#include <nlohmann/json.hpp>
#include "detail/NodeBase.h"

namespace nv {
	namespace detail {
		struct DynamicNodeTraits {
			using String = std::string;

			using Node = BufferedNode*;
			using Polygon = DynamicPolygon;

			template<typename T>
			using ObjectGroup = plf::hive<T>;

			using Layer = std::tuple<
				plf::hive<Texture>, plf::hive<DynamicPolygon>, plf::hive<Node>
			>;
			using Layers = nv::detail::ObjectLayers<std::deque<Layer>>;
			using LayerMap = boost::unordered_flat_map<std::string, Layer*>;

			template<typename Key, typename Value>
			using ObjectLookupMap = boost::unordered_flat_map<Key, Value*>;

			using ObjectLookups = std::tuple<
				ObjectLookupMap<std::string, Texture>,
				ObjectLookupMap<std::string, BufferedPolygon>,
				ObjectLookupMap<std::string, BufferedNode>
			>;
		};

		static_assert(NodeTraits<DynamicNodeTraits>);
	}

	class DynamicNode : public detail::NodeBase<detail::DynamicNodeTraits> {
	private:
		DynamicNode() = default;
	public:
		friend struct nlohmann::adl_serializer<DynamicNode>;
	};
}