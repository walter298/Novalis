#pragma once

#include <nlohmann/json.hpp>

#include "ObjectGroupManager.h"
#include "Layer.h"

namespace nv {
	namespace editor {
		struct NodeSerializationResult {
			nlohmann::json json;
			size_t byteCount = 0;
		};
		NodeSerializationResult createNodeJson(const std::vector<Layer>& layers, const ObjectGroupManager& objectGroups);
	}
}