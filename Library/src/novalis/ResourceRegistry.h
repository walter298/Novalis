#pragma once

#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

#include "BufferedNode.h"
#include "DynamicNode.h"

namespace nv {
	class ResourceRegistry {
	public:
		template<typename Node>
		using Map = boost::unordered_flat_map<std::filesystem::path, Node>;
	private:
		using BufferedNodeMap = Map<std::unique_ptr<BufferedNode>>;
		using DynamicNodeMap  = Map<std::unique_ptr<DynamicNode>>;
		using TextureMap      = Map<detail::TexturePtr>;

		BufferedNodeMap m_bufferedNodeMap;
		DynamicNodeMap m_dynamicNodeMap;
		TextureMap m_textureMap;
	public:
		detail::TexturePtr loadTexture(SDL_Renderer* renderer, const std::filesystem::path& path);
		BufferedNode loadBufferedNode(const std::filesystem::path& path);
		BufferedNode loadBufferedNode(const std::filesystem::path& path, const nlohmann::json& nodeJson);
		DynamicNode loadDynamicNode(const std::filesystem::path& path);
	};
}

