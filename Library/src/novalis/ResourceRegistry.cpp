#include "ResourceRegistry.h"

#include "detail/serialization/BufferedNodeSerialization.h"
#include "detail/serialization/DynamicNodeSerialization.h"

nv::TexturePtr nv::ResourceRegistry::loadTexture(SDL_Renderer* renderer, const std::string& path) {
	auto texIt = m_textureMap.find(path);
	if (texIt == m_textureMap.end()) {
		TexturePtr tex{ renderer, path.c_str() };
		m_textureMap.emplace(path, tex);
		return tex;
	} else {
		return texIt->second;
	}
}

template<typename Node>
Node loadNodeImpl(nv::ResourceRegistry::Map<std::unique_ptr<Node>>& nodeMap, const std::string& path) {
	auto nodeIt = nodeMap.find(path);
	if (nodeIt == nodeMap.end()) {
		std::ifstream file{ path };
		assert(file.is_open());
		auto nodeJson = nlohmann::json::parse(file);
		auto nodePtr = std::make_unique<Node>(nodeJson.get<Node>());
		auto& node = *nodePtr;
		nodeMap.emplace(path, std::move(nodePtr));
		return node;
	} else {
		return *nodeIt->second;
	}
}

nv::BufferedNode nv::ResourceRegistry::loadBufferedNode(const std::string& path) {
	return loadNodeImpl(m_bufferedNodeMap, path);
}
nv::DynamicNode nv::ResourceRegistry::loadDynamicNode(const std::string& path) {
	return loadNodeImpl(m_dynamicNodeMap, path);
}
