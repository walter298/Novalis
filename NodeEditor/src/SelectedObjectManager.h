#include "EditedObjectData.h"

namespace nv {
	namespace editor {
		class SelectedObjectManager {
		private:
			template<typename Object>
			struct SelectedObjectData {
				EditedObjectData<Object>* obj = nullptr;
				EditedObjectHive<Object>* objLayer = nullptr;
				EditedObjectHive<Object>::iterator it;
			};

			using SelectedObjectVariant = std::variant<
				std::monostate,
				SelectedObjectData<DynamicPolygon>,
				SelectedObjectData<Texture>,
				SelectedObjectData<BufferedNode>
			>;
			SelectedObjectVariant m_selectedObject = std::monostate{};


		};
	}
}