#pragma once

#include "ObjectGroupManager.h"
#include "Layer.h"

namespace nv {
	namespace editor {
		std::string createNodeJson(const std::vector<Layer>& layers, std::string_view nodeName, ObjectGroupManager& objectGroups);
	}
}