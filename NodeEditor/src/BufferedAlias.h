#pragma once

#include <type_traits>
#include <novalis/Polygon.h>

namespace nv {
	namespace editor {
		template<typename Object>
		using BufferedObject = std::conditional_t<
			std::same_as<Object, DynamicPolygon>,
			BufferedPolygon,
			Object
		>;
	}
}