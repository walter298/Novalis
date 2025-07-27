#pragma once

#include <nlohmann/json.hpp>

#include <boost/container/flat_map.hpp>
#include <novalis/BufferedNode.h>
#include <novalis/detail/serialization/AutoSerialization.h>

#include "FileID.h"

namespace nv {
	namespace editor {
		struct NodeLayout {
			struct NodeRegion {
				size_t offset = 0;
				size_t byteCount = 0;
			};
			using Nodes = boost::container::flat_map<FileID, NodeRegion>;
			Nodes nodes;
			size_t childNodeByteCount = 0;
			size_t totalByteCount = 0; //total byte count (not just of child nodes)

			void resize(FileID nodeId, size_t newByteCount) noexcept;

			friend class nlohmann::adl_serializer<NodeLayout>;
		};
	}
}

namespace nlohmann {
	template<>
	struct adl_serializer<nv::editor::NodeLayout> {
		static constexpr const char* NODE_REGIONS_KEY = "Node_Regions";
		static constexpr const char* CHILD_NODE_BYTE_COUNT = "Child_Node_Byte_Count";
		static constexpr const char* BYTE_COUNT_KEY = "Byte_Count";

		static void to_json(json& j, const nv::editor::NodeLayout& nodeLayout) {
			auto& nodeRegions = j[NODE_REGIONS_KEY] = json::array();
			for (const auto& [id, region] : nodeLayout.nodes) {
				nodeRegions[id] = region;
			}
			j[CHILD_NODE_BYTE_COUNT] = nodeLayout.childNodeByteCount;
			j[BYTE_COUNT_KEY] = nodeLayout.totalByteCount;
		}

		static void from_json(const json& j, nv::editor::NodeLayout& nodeLayout) {
			nodeLayout.nodes.clear();
			for (const auto& [idJson, regionJson] : j.items()) {
				nodeLayout.nodes.emplace(
					std::stoi(idJson),
					regionJson.get<nv::editor::NodeLayout::NodeRegion>()
				);
			}
			nodeLayout.childNodeByteCount = j[CHILD_NODE_BYTE_COUNT].get<size_t>();
			nodeLayout.totalByteCount     = j[BYTE_COUNT_KEY].get<size_t>();
		}
	};
}