#include "EditedObjectData.h"
#include "EditedObjectGroup.h"

namespace nv {
	namespace editor {
		template<typename Object>
		json makeObjectJson(json& j, const EditedObjectData<Object>& obj) {
			json j;

			auto& metadataJson = j[METADATA_KEY];
			auto& objectJson   = j[OBJECT_KEY];

			metadataJson[NAME_KEY] = obj.name;
			auto& objectGroupsJson = metadataJson[OBJECT_GROUP_KEY] = json::array();
			for (const auto& id : obj.groupIDs) {
				const auto& objectGroup = objectGroups.getGroup(id);
				objectGroupsJson.push_back(objectGroup.name);
			}

			if constexpr (std::same_as<Object, BufferedNode>) {
				metadataJson[PATH_KEY] = obj.filePath;
				metadataJson[OPACITY_KEY] = obj.obj.getOpacity();
				metadataJson[SCREEN_SCALE_KEY] = obj.obj.getScreenScale();
				metadataJson[SCREEN_POS_KEY] = obj.obj.getScreenPos();
				metadataJson[WORLD_POS_KEY] = obj.obj.getWorldPos();
			} else if constexpr (std::same_as<Object, Texture>) {
				objectJson[RENDER_DATA_KEY] = obj.obj.texData;
				objectJson[IMAGE_PATH_KEY] = obj.texPath;
			} else {
				objectJson = obj.obj;
			}

			return j;
		}
	}
}