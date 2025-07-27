#pragma once

#include <deque>
#include <boost/unordered/unordered_flat_map.hpp>

#include <nlohmann/json.hpp>
#include "detail/NodeBase.h"

namespace nv {
	namespace detail {
		struct DynamicNodeTraits {
			using String = std::string;

			using Node = BufferedNode*;
			using Polygon = DynamicPolygon;

			template<typename T>
			using ObjectStorage = plf::hive<T>;

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

			using ObjectGroup = std::tuple<
				std::vector<BufferedNode*>, 
				std::vector<DynamicPolygon*>, 
				std::vector<Texture*>
			>;

			using ObjectGroupMap = boost::unordered_flat_map<std::string, ObjectGroup>;
		};

		static_assert(NodeTraits<DynamicNodeTraits>);
	}

	class DynamicNode : public detail::NodeBase<detail::DynamicNodeTraits> {
	private:
	public:
		DynamicNode() = default;
		//DynamicNode(const DynamicNode&) = delete; //disallow shallow copying

		//void addLayer(std::string layerName) {
		//	assert(!m_layerMap.contains(layerName));
		//	m_layerMap.emplace(std::move(layerName), Layer{});
		//}

		//template<typename Object>
		//void addObject(const std::string& layerName, Object&& object) {
		//	auto& layer = m_layerMap.at(layerName);
		//	auto& objectStorage = std::get<ObjectStorage<std::remove_cvref_t<Object>>>(layer);
		//	objectStorage.insert(std::forward<Object>(object));
		//}

		friend struct nlohmann::adl_serializer<DynamicNode>;
	};
}