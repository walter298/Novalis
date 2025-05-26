#include "EditedObjectData.h"

namespace nv {
	namespace editor {
		template<typename Object>
		using ObjectLookup = boost::unordered_flat_set<EditedObjectData<Object>*>;

		struct EditedObjectGroup {
			std::string name;

			enum SyncOption : bool { 
				Unsynced = false,
				Synced = true
			};

			SyncOption positionSynced = SyncOption::Synced;
			SyncOption rotationSynced = SyncOption::Synced;
			SyncOption scaleSynced    = SyncOption::Synced;
			SyncOption opacitySynced  = SyncOption::Synced;

			using Objects = std::tuple<
				ObjectLookup<DynamicPolygon>,
				ObjectLookup<Texture>,
				ObjectLookup<BufferedNode>
			>;

			Objects objects;

			EditedObjectGroup()                             = default;
			EditedObjectGroup(const EditedObjectGroup&)     = delete;
			EditedObjectGroup(EditedObjectGroup&&) noexcept = default;

			template<typename Object>
			void addObject(EditedObjectData<Object>* obj) {
				std::get<boost::unordered_flat_set<EditedObjectData<Object>*>>(objects).insert(obj);
			}
			template<typename Object>
			void removeObject(EditedObjectData<Object>* obj) {
				std::get<boost::unordered_flat_set<EditedObjectData<Object>*>>(objects).erase(obj);
			}
		};

		namespace bg = boost::geometry;

		class EditedObjectGroupManager {
		private:
			boost::unordered_flat_map<ID<EditedObjectGroup>, EditedObjectGroup> m_objectGroups;
		public:
			std::pair<const ID<EditedObjectGroup>, EditedObjectGroup&> addGroup() {
				auto [it, inserted] = m_objectGroups.emplace(
					std::piecewise_construct, std::forward_as_tuple(), std::forward_as_tuple()
				);
				return { it->first, it->second };
			}
			void removeGroup(ID<EditedObjectGroup> id) {
				nv::detail::forEachDataMember([&](auto& objects) {
					for (auto& object : objects) {
						object->groupIDs.erase(id);
					}
					return nv::detail::STAY_IN_LOOP;
				}, m_objectGroups.at(id).objects);
				m_objectGroups.erase(id);
			}
		private:
			template<typename Object>
			EditedObjectGroup::Objects getActedOnObjects(EditedObjectData<Object>& object, EditedObjectGroup::SyncOption EditedObjectGroup::* syncOption) {
				EditedObjectGroup::Objects actedOnObjects;
				boost::unordered_flat_set<ID<EditedObjectGroup>> syncedObjectGroups;
				std::queue<ID<EditedObjectGroup>> unsyncedObjectGroups;

				std::get<ObjectLookup<Object>>(actedOnObjects).insert(&object);
				unsyncedObjectGroups.push_range(object.groupIDs);

				while (!unsyncedObjectGroups.empty()) {
					auto unsyncedObjectID = unsyncedObjectGroups.front();
					unsyncedObjectGroups.pop();
					syncedObjectGroups.insert(unsyncedObjectID);

					auto& group = m_objectGroups.at(unsyncedObjectID);
					if (group.*syncOption != EditedObjectGroup::Synced) {
						continue;
					}

					nv::detail::forEachDataMember([&]<typename Object>(ObjectLookup<Object>& objects) { //objects is a pointer
						for (EditedObjectData<Object>* object : objects) {
							//if object has been acted on already, continue
							auto& actedOnObjectLookup = std::get<ObjectLookup<Object>>(actedOnObjects);
							if (actedOnObjectLookup.contains(object)) {
								continue;
							}

							//mark object as acted on and then invoke function with it
							actedOnObjectLookup.insert(object);
							
							//if object is present in any other object groups, other objects in those may also need to be synced up
							unsyncedObjectGroups.push_range(object->groupIDs | std::views::filter([&, this](const auto& objectGroupID) {
								return !syncedObjectGroups.contains(objectGroupID);
							}));
						}
						return nv::detail::STAY_IN_LOOP;
					}, group.objects);
				}

				return actedOnObjects;
			}
		public:
			template<typename Object>
			void scale(EditedObjectData<Object>& scaledObject, float scale) {
				auto actedOnObjects = getActedOnObjects(scaledObject, &EditedObjectGroup::scaleSynced);
				
				nv::detail::forEachDataMember([&](auto& objects) {
					for (auto object : objects) { //object is a pointer, so no copy
						object->obj.screenScale(scale);
						object->obj.worldScale(scale);
					}
					return nv::detail::STAY_IN_LOOP;
				}, actedOnObjects);
			}
			void rotate(ID<EditedObjectGroup> id, float angle) {
				//forEachImpl(id, &EditedObjectGroup::rotationSynced, [angle](auto& object) {
				//	//object.obj.rotate(angle);
				//});
			}
			template<typename Object>
			void move(EditedObjectData<Object>& movedObject, Point delta) {
				auto actedOnObjects = getActedOnObjects(movedObject, &EditedObjectGroup::positionSynced);
				nv::detail::forEachDataMember([&](auto& objects) {
					for (auto object : objects) { //object is a pointer, so no copy
						object->obj.screenMove(delta);
						object->obj.worldMove(delta);
					}
					return nv::detail::STAY_IN_LOOP;
				}, actedOnObjects);
			}

			template<typename Object>
			void setOpacity(EditedObjectData<Object>& opacifiedObject, uint8_t opacity) {
				auto actedOnObjects = getActedOnObjects(opacifiedObject, &EditedObjectGroup::opacitySynced);
				nv::detail::forEachDataMember([&](auto& objects) {
					for (auto object : objects) { //object is a pointer, so no copy
						object->obj.setOpacity(opacity);
					}
					return nv::detail::STAY_IN_LOOP;
				}, actedOnObjects);
			}
			decltype(auto) getGroup(this auto&& self, ID<EditedObjectGroup> id) {
				return self.m_objectGroups.at(id);
			}
		};
	}
}