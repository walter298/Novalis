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
		using Map = boost::unordered_flat_map<std::string, Node>;
	private:
		using BufferedNodeMap = Map<std::unique_ptr<BufferedNode>>;
		using DynamicNodeMap  = Map<std::unique_ptr<DynamicNode>>;
		using TextureMap      = Map<nv::TexturePtr>;

		BufferedNodeMap m_bufferedNodeMap;
		DynamicNodeMap m_dynamicNodeMap;
		TextureMap m_textureMap;
	public:
		TexturePtr loadTexture(SDL_Renderer* renderer, const std::string& path);
		BufferedNode loadBufferedNode(const std::string& path);
		DynamicNode loadDynamicNode(const std::string& path);
	};
}

