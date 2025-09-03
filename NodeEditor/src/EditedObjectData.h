#pragma once

#include <boost/unordered/unordered_flat_set.hpp>
#include <novalis/ID.h>
#include <novalis/detail/serialization/AutoSerialization.h>
#include <novalis/detail/serialization/KeyConstants.h>
#include <novalis/detail/serialization/PolygonSerialization.h>
#include <novalis/detail/serialization/SpritesheetSerialization.h>
#include <novalis/detail/serialization/TextureSerialization.h>
#include <novalis/BufferedNode.h>

#include "File.h"
#include "NameManager.h"

namespace nv {
	namespace editor {
		using namespace nv::detail::json_constants;

		struct EditedObjectGroup;

		static NameManager objectNameManager{ "Unnamed Object" };

		template<typename Object>
		class EditedObjectDataBase {
		private:
			std::string m_name;
		public:
			Object obj;
			uint8_t opacity = 255;
			float angle = 0.0f;
			float scale = 1.0f;
			ID<void> id;
			boost::unordered_flat_set<ID<EditedObjectGroup>> groupIDs;

			template<typename... Args>
			constexpr EditedObjectDataBase(Args&&... args) requires(std::constructible_from<Object, Args...>)
				: obj{ std::forward<Args>(args)... }
			{
				objectNameManager.makeExistingNameUnique(m_name);
			}

			EditedObjectDataBase(const EditedObjectDataBase&) = default;
			EditedObjectDataBase(EditedObjectDataBase&&) noexcept = default;

			void loadName(std::string name) {
				m_name = std::move(name);
				objectNameManager.makeExistingNameUnique(m_name);
			}

			void inputName(NameManager& nameManager) {
				nameManager.inputName("Object Name", m_name);
			}

			const std::string& getName() const noexcept {
				return m_name;
			}

			void destroy() {
				objectNameManager.deleteName(m_name);
			}
		};

		template<typename Object>
		struct ObjectMetadata : public EditedObjectDataBase<Object> {
			template<typename... Args>
			constexpr ObjectMetadata(Args&&... args) requires(std::constructible_from<Object, Args...>)
				: EditedObjectDataBase<Object>{ std::forward<Args>(args)... }
			{
			}

			static ObjectMetadata<Object> load(const json& objectJson) {
				ObjectMetadata<Object> ret{ objectJson[OBJECT_KEY].get<Object>()};
				ret.loadName(objectJson[METADATA_KEY][NAME_KEY].get<std::string>());
				return ret;
			}
		};

		template<typename T>
		static ObjectMetadata<T> loadImageObject(const json& j) {
			ObjectMetadata<T> ret{ j[OBJECT_KEY].get<T>() };
			ret.loadName(j[METADATA_KEY][NAME_KEY].get<std::string>());
			ret.texPath = j[OBJECT_KEY][IMAGE_PATH_KEY].get<std::string>();
			ret.texFile = j[OBJECT_KEY][IMAGE_FILE_ID_KEY].get<FileID>();
			return ret;
		}

		template<>
		struct ObjectMetadata<Texture> : public EditedObjectDataBase<Texture> {
			std::string texPath;
			FileID texFile;

			template<typename... Args>
			constexpr ObjectMetadata(Args&&... args) requires(std::constructible_from<Texture, Args...>)
				: EditedObjectDataBase<Texture>{ std::forward<Args>(args)... }
			{
			}

			static ObjectMetadata<Texture> load(const json& objectJson) {
				return loadImageObject<Texture>(objectJson);
			}
		};

		template<>
		struct ObjectMetadata<Spritesheet> : public EditedObjectDataBase<Spritesheet> {
			std::filesystem::path texPath;
			FileID texFile;

			template<typename... Args>
			constexpr ObjectMetadata(Args&&... args) requires(std::constructible_from<Spritesheet, Args...>)
				: EditedObjectDataBase<Spritesheet>{ std::forward<Args>(args)... }
			{
			}

			static ObjectMetadata<Spritesheet> load(const json& objectJson) {
				return loadImageObject<Spritesheet>(objectJson);
			}
		};

		template<>
		struct ObjectMetadata<BufferedNode> : public EditedObjectDataBase<BufferedNode> {
			std::string filePath;

			template<typename... Args>
			constexpr ObjectMetadata(Args&&... args) requires(std::constructible_from<BufferedNode, Args...>)
				: EditedObjectDataBase<BufferedNode>{ std::forward<Args>(args)... }
			{
			}

			static ObjectMetadata<BufferedNode> load(const json& objectJson) {
				auto path = objectJson[METADATA_KEY][PATH_KEY].get<std::string>();
				ObjectMetadata<BufferedNode> ret{
					getGlobalInstance()->registry.loadBufferedNode(path)
				};
				ret.filePath = path;
				ret.loadName(objectJson[METADATA_KEY][NAME_KEY].get<std::string>());
				ret.obj.setOpacity(objectJson[METADATA_KEY][OPACITY_KEY].get<uint8_t>());
				ret.obj.screenScale(objectJson[METADATA_KEY][SCREEN_SCALE_KEY].get<float>());
				ret.obj.setScreenPos(objectJson[METADATA_KEY][SCREEN_POS_KEY].get<Point>());
				ret.obj.setWorldPos(objectJson[METADATA_KEY][WORLD_POS_KEY].get<Point>());

				return ret;
			}
		};

		template<typename Object>
		using EditedObjectHive = plf::hive<ObjectMetadata<Object>>;
	}
}