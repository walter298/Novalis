#pragma once

#include <boost/unordered/unordered_flat_set.hpp>
#include <novalis/ID.h>
#include <novalis/detail/serialization/AutoSerialization.h>
#include <novalis/detail/serialization/KeyConstants.h>
#include <novalis/detail/serialization/PolygonSerialization.h>
#include <novalis/detail/serialization/SpritesheetSerialization.h>
#include <novalis/detail/serialization/TextureSerialization.h>
#include <novalis/BufferedNode.h>

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
		protected:
			void loadName(std::string name) {
				m_name = std::move(name);
				objectNameManager.makeNewName(m_name);
			}
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
				objectNameManager.makeNewName(m_name);
			}

			EditedObjectDataBase(const EditedObjectDataBase&) = delete;
			EditedObjectDataBase(EditedObjectDataBase&&) noexcept = default;

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
		struct EditedObjectData : public EditedObjectDataBase<Object> {
			template<typename... Args>
			constexpr EditedObjectData(Args&&... args) requires(std::constructible_from<Object, Args...>)
				: EditedObjectDataBase<Object>{ std::forward<Args>(args)... }
			{
			}

			static EditedObjectData<Object> load(const json& objectJson) {
				EditedObjectData<Object> ret{ objectJson[OBJECT_KEY].get<Object>()};
				ret.loadName(objectJson[METADATA_KEY][NAME_KEY].get<std::string>());
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
				EditedObjectData<Texture> ret{ objectJson[OBJECT_KEY].get<Texture>() };
				ret.loadName(objectJson[METADATA_KEY][NAME_KEY].get<std::string>());
				ret.texPath = objectJson[OBJECT_KEY][IMAGE_PATH_KEY].get<std::string>();

				return ret;
			}
		};

		template<>
		struct EditedObjectData<Spritesheet> : public EditedObjectDataBase<Spritesheet> {
			std::string texPath;

			template<typename... Args>
			constexpr EditedObjectData(Args&&... args) requires(std::constructible_from<Spritesheet, Args...>)
				: EditedObjectDataBase<Spritesheet>{ std::forward<Args>(args)... }
			{
			}

			static EditedObjectData<Spritesheet> load(const json& objectJson) {
				EditedObjectData<Spritesheet> ret{ objectJson[OBJECT_KEY].get<Spritesheet>() };
				ret.loadName(objectJson[METADATA_KEY][NAME_KEY].get<std::string>());
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
				std::println("{}", objectJson.dump(2));
				auto path = objectJson[METADATA_KEY][PATH_KEY].get<std::string>();
				EditedObjectData<BufferedNode> ret{
					getGlobalInstance()->registry.loadBufferedNode(path)
				};
				ret.loadName(objectJson[METADATA_KEY][NAME_KEY].get<std::string>());
				ret.filePath = path;
				ret.obj.setOpacity(objectJson[METADATA_KEY][OPACITY_KEY].get<uint8_t>());
				ret.obj.screenScale(objectJson[METADATA_KEY][SCREEN_SCALE_KEY].get<float>());
				ret.obj.setScreenPos(objectJson[METADATA_KEY][SCREEN_POS_KEY].get<Point>());
				ret.obj.setWorldPos(objectJson[METADATA_KEY][WORLD_POS_KEY].get<Point>());

				return ret;
			}
		};

		template<typename Object>
		using EditedObjectHive = plf::hive<EditedObjectData<Object>>;
	}
}