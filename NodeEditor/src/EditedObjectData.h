#pragma once

#include <boost/unordered/unordered_flat_set.hpp>
#include <novalis/ID.h>
#include <novalis/detail/serialization/KeyConstants.h>
#include <novalis/detail/serialization/TextureSerialization.h>
#include <novalis/Node.h>

namespace nv {
	namespace editor {
		using namespace nv::detail::json_constants;

		struct EditedObjectGroup;

		template<typename Object>
		struct EditedObjectDataBase {
			Object obj;
			std::string name;
			uint8_t opacity = 255;
			float scale = 1.0f;
			boost::unordered_flat_set<ID<EditedObjectGroup>> groupIDs;

			template<typename... Args>
			constexpr EditedObjectDataBase(Args&&... args) requires(std::constructible_from<Object, Args...>)
				: obj{ std::forward<Args>(args)... }
			{
			}
		};

		template<typename Object>
		struct EditedObjectData : public EditedObjectDataBase<Object> {
			template<typename... Args>
			constexpr EditedObjectData(Args&&... args) requires(std::constructible_from<Object, Args...>)
				: EditedObjectDataBase<Object>{ std::forward<Args>(args)... }
			{
			}

			static EditedObjectData<Object> load(const json& objectJson) {
				EditedObjectData<Object> ret{ objectJson[OBJECT_KEY].get<Object>()};
				ret.name = objectJson[METADATA_KEY][NAME_KEY].get<std::string>();
				return ret;
			}
		};

		template<>
		struct EditedObjectData<Texture> : public EditedObjectDataBase<Texture> {
			std::string texPath;

			template<typename... Args>
			constexpr EditedObjectData(Args&&... args) requires(std::constructible_from<Texture, Args...>)
				: EditedObjectDataBase<Texture>{ std::forward<Args>(args)... }
			{
			}

			static EditedObjectData<Texture> load(const json& objectJson) {
				EditedObjectData<Texture> ret{ objectJson[OBJECT_KEY].get<Texture>()};
				ret.name = objectJson[METADATA_KEY][NAME_KEY].get<std::string>();
				ret.texPath = objectJson[OBJECT_KEY][IMAGE_PATH_KEY].get<std::string>();

				return ret;
			}
		};

		template<>
		struct EditedObjectData<BufferedNode> : public EditedObjectDataBase<BufferedNode> {
			std::string filePath;

			template<typename... Args>
			constexpr EditedObjectData(Args&&... args) requires(std::constructible_from<BufferedNode, Args...>)
				: EditedObjectDataBase<BufferedNode>{ std::forward<Args>(args)... }
			{
			}

			static EditedObjectData<BufferedNode> load(const json& objectJson) {
				auto path = objectJson[METADATA_KEY][PATH_KEY].get<std::string>();
				EditedObjectData<BufferedNode> ret{
					getGlobalInstance()->registry.loadBufferedNode(path)
				};
				ret.name = objectJson[METADATA_KEY][NAME_KEY].get<std::string>();
				ret.filePath = path;
				ret.obj.setOpacity(objectJson[METADATA_KEY][OPACITY_KEY].get<uint8_t>());
				ret.obj.screenScale(objectJson[METADATA_KEY][SCREEN_SCALE_KEY].get<float>());
				ret.obj.setScreenPos(objectJson[METADATA_KEY][SCREEN_POS_KEY].get<Point>());
				ret.obj.setWorldPos(objectJson[METADATA_KEY][WORLD_POS_KEY].get<Point>());
				
				return ret;
			}
		};

		template<typename Object>
		void to_json(json& j, const EditedObjectData<Object>& obj) {
			auto& metadataJson = j[METADATA_KEY];
			auto& objectJson   = j[OBJECT_KEY];

			metadataJson[NAME_KEY] = obj.name;

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
		}

		template<typename Object>
		using EditedObjectHive = plf::hive<EditedObjectData<Object>>;
	}
}