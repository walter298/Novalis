//#include <fstream>
//#include <nlohmann/json.hpp>
//#include <novalis/detail/serialization/AutoSerialization.h>
//#include <novalis/detail/serialization/BufferedNodeSerialization.h>
//
//#include "ProjectGraph.h"
//#include "NodeSerialization.h"
//
//constexpr const char* REFERENCE_MAP_KEY = "Reference_Map";
//constexpr const char* NODE_ID_KEY = "ID";
//constexpr const char* NODE_KEY = "Layout";
//constexpr const char* FILE_ID_OFFSET_KEY = "File_ID_Offset";
//constexpr const char* NODE_ID_OFFSET_KEY = "Node_ID_Offset";
//
//bool nv::editor::ProjectGraph::isParent(FileID childID, FileID parentID) const {
//	assert(m_nodes.contains(parentID));
//
//	const auto& childNode = m_nodes.at(childID);
//	if (childNode.parentIDs.contains(parentID)) {
//		return true;
//	}
//	for (const auto& immediateParentName : childNode.parentIDs) {
//		if (isParent(immediateParentName, parentID)) {
//			return true;
//		}
//	}
//	return false;
//}
//
//using NodeSerializer = nlohmann::adl_serializer<nv::BufferedNode>;
//
//void nv::editor::ProjectGraph::updateParentSizes(FileID parentID, FileID resizedNodeID, size_t byteCount) {
//	auto& [parentParentIDs, parentLayout, json] = m_nodes.at(parentID);
//	parentLayout.resize(resizedNodeID, byteCount);
//	json[BYTES_KEY] = parentLayout.totalByteCount;
//	json[NodeSerializer::typeSizeKey<std::byte>()] = parentLayout.childNodeByteCount;
//	for (const auto& parentParentID : parentParentIDs) {
//		updateParentSizes(parentParentID, parentID, parentLayout.totalByteCount);
//	}
//}
//
//void nv::editor::ProjectGraph::addNewNode(FileID nodeID) {
//	assert(!m_nodes.contains(nodeID));
//	m_nodes.emplace(nodeID, Node{}); //todo: implement json!
//}
//
//void nv::editor::ProjectGraph::deleteNode(FileID id) {
//	assert(m_nodes.at(id).parentIDs.empty()); //should be checked within the virtual filesystem
//	m_nodes.erase(id);
//}
//
//void nv::editor::ProjectGraph::saveNode(NodeEditor& nodeEditor, ErrorPopup& errorPopup) {
//	auto id = nodeEditor.getID();
//	auto& node = m_nodes.at(id);
//
//	//create the node json
//	auto [nodeJson, byteCount] = nodeEditor.serialize();
//	node.json = std::move(nodeJson);
//	node.layout.totalByteCount = byteCount;
//
//	for (auto& parentID : node.parentIDs) {
//		updateParentSizes(parentID, id, byteCount);
//	}
//}
//
//void nv::editor::ProjectGraph::save() const {
//	nlohmann::json root;
//	auto& jsonRefList = root[REFERENCE_MAP_KEY] = nlohmann::json::array();
//
//	for (const auto& [id, node] : m_nodes) {
//		auto& nodeJson = jsonRefList.emplace_back();
//		nodeJson[NODE_ID_KEY] = id;
//		nodeJson[NODE_KEY] = node;
//	}
//
//	/*root[FILE_ID_OFFSET_KEY] = ID<FileTag>::IDCount;
//	root[NODE_ID_OFFSET_KEY] = FileID::IDCount;*/
//
//	/*std::ofstream file{ m_rootDirectory };
//	file << root.dump(2);*/
//}
