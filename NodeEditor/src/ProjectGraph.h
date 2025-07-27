//#pragma once
//
//#include <string>
//#include <vector>
//#include <boost/unordered/unordered_flat_map.hpp>
//#include <boost/unordered/unordered_flat_set.hpp>
//
//#include "ErrorPopup.h"
//#include "NodeEditor.h"
//#include "NodeLayout.h"
//#include "FileID.h"
//
//namespace nv {
//	namespace editor {
//		class ProjectGraph {
//		private:
//			struct Node {
//				boost::unordered_flat_set<FileID> parentIDs;
//				NodeLayout layout;
//				nlohmann::json json;
//			};
//		
//			boost::unordered_flat_map<FileID, Node> m_nodes; 
//			
//			void updateParentSizes(FileID parentID, FileID resizedNodeID, size_t byteCount);
//
//			bool isParent(FileID childID, FileID parentID) const;
//		public:
//			void addChildNode(FileID childID, FileID parentID);
//			void addNewNode(FileID nodeID);
//			void deleteNode(FileID id);
//			void saveNode(NodeEditor& node, ErrorPopup& errorPopup);
//			void save() const;
//
//			friend class nlohmann::adl_serializer<ProjectGraph>;
//		};
//	}
//}
//
//namespace nlohmann {
//	template<>
//	struct adl_serializer<nv::editor::ProjectGraph> {
//		static constexpr const char* REFERENCE_MAP_KEY = "Reference_Map";
//		static constexpr const char* NODE_ID_KEY = "ID";
//		static constexpr const char* NODE_KEY = "Node";
//		
//		static void from_json(const json& j, nv::editor::ProjectGraph& projectGraph) {
//			const auto& jsonRefList = j[REFERENCE_MAP_KEY];
//			for (const auto& nodeJson : jsonRefList) {
//				auto id = nodeJson[NODE_ID_KEY].get<nv::editor::FileID>();
//				auto node = nodeJson[NODE_KEY].get<nv::editor::ProjectGraph::Node>();
//				projectGraph.m_nodes.emplace(id, std::move(node));
//			}
//		}
//
//		static void to_json(json& j, const nv::editor::ProjectGraph& projectGraph) {
//			nlohmann::json root;
//			auto& jsonRefList = root[REFERENCE_MAP_KEY] = nlohmann::json::array();
//
//			for (const auto& [id, node] : projectGraph.m_nodes) {
//				auto& nodeJson = jsonRefList.emplace_back();
//				nodeJson[NODE_ID_KEY] = id;
//				nodeJson[NODE_KEY] = node;
//			}
//		}
//	};
//}