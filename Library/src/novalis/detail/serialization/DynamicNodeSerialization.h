#include "KeyConstants.h"
#include "../../DynamicNode.h"
#include "../reflection/TypeInfo.h"
#include "../serialization/PolygonSerialization.h"
#include "../serialization/TextureSerialization.h"

namespace nlohmann {
	template<>
	struct adl_serializer<nv::DynamicNode> {
		static nv::DynamicNode from_json(const json& nodeJson) {
			using namespace nv::detail::json_constants;

			nv::DynamicNode ret;

			for (const auto& layerJson : nodeJson[LAYERS_KEY]) {
				auto& currLayer = ret.m_objectLayers.storage.emplace_back();

				//add lookup for current layer
				auto layerName = layerJson[NAME_KEY].get<std::string>();
				if (layerName.empty()) {
					ret.m_layerMap.emplace(std::move(layerName), &currLayer);
				}
				
				nv::detail::forEachDataMember([&]<typename Object>(plf::hive<Object>& objectGroup) {
					using BufferedObjectKey = std::conditional_t<
						std::same_as<Object, nv::DynamicPolygon>, nv::BufferedPolygon, Object
					>;

					auto typeName = nv::detail::getTypeName<BufferedObjectKey>();
					auto objectGroupJsonIt = layerJson.find(typeName);

					if (objectGroupJsonIt == layerJson.end()) {
						return nv::detail::STAY_IN_LOOP;
					}

					auto& objectGroupJson = *objectGroupJsonIt;
					for (const auto& objectJson : objectGroupJson) {
						if constexpr (!std::same_as<Object, nv::BufferedNode*> && !std::same_as<Object, nv::BufferedNode>) {
							objectGroup.insert(objectJson[OBJECT_KEY].get<Object>());
						}
					}
					return nv::detail::STAY_IN_LOOP;
				}, currLayer);
			}

			return ret;
		}
	};
}