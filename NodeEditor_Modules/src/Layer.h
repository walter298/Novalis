#pragma once

#include <string>

#include "EditedObjectData.h"

namespace nv {
	namespace editor {
		struct Layer {
			std::string name;
			using Objects = std::tuple<
				EditedObjectHive<Texture>,
				EditedObjectHive<BufferedNode>,
				EditedObjectHive<DynamicPolygon>
			>;
			Objects objects;
		};
	}
}