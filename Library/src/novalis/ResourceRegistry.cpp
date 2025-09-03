#include "ResourceRegistry.h"

#include "detail/serialization/BufferedNodeSerialization.h"
#include "detail/serialization/DynamicNodeSerialization.h"

nv::detail::TexturePtr nv::ResourceRegistry::loadTexture(SDL_Renderer* renderer, 
	const std::filesystem::path& path) 
{
	auto texIt = m_textureMap.find(path);
	if (texIt == m_textureMap.end()) {
		auto pathStr = path.string();
		detail::TexturePtr tex{ renderer, pathStr.c_str() };
		m_textureMap.emplace(path, tex);
		return tex;
	} else {
		return texIt->second;
	}
}

namespace {
	template<typename Node>
	Node loadNodeImpl(nv::ResourceRegistry::Map<std::unique_ptr<Node>>& nodeMap, const std::filesystem::path& path) {
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
}

nv::BufferedNode nv::ResourceRegistry::loadBufferedNode(const std::filesystem::path& path) {
	return loadNodeImpl(m_bufferedNodeMap, path);
}

nv::BufferedNode nv::ResourceRegistry::loadBufferedNode(const std::filesystem::path& path, 
	const nlohmann::json& nodeJson) 
{
	auto nodePtr = std::make_unique<BufferedNode>(nodeJson.get<BufferedNode>());
	auto& node = *nodePtr;
	m_bufferedNodeMap.emplace(path, std::move(nodePtr));
	return node;
}

nv::DynamicNode nv::ResourceRegistry::loadDynamicNode(const std::filesystem::path& path) {
	return loadNodeImpl(m_dynamicNodeMap, path);
}