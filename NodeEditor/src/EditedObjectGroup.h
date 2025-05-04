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

		class EditedObjectGroupManager {
		private:
			boost::unordered_flat_map<ID<EditedObjectGroup>, EditedObjectGroup> m_objectGroups;
		public:
			void addGroup(ID<EditedObjectGroup> id, EditedObjectGroup&& group) {
				m_objectGroups.emplace(id, std::move(group));
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
			template<typename Object, typename Func>
			void forEachImpl(EditedObjectData<Object>& object, EditedObjectGroup::SyncOption EditedObjectGroup::* syncOption, Func func) {
				EditedObjectGroup::Objects consumedObjects;
				boost::unordered_flat_set<ID<EditedObjectGroup>> syncedObjectGroups;	
				std::queue<ID<EditedObjectGroup>> unsyncedObjectGroups;

				func(object);
				std::get<ObjectLookup<Object>>(consumedObjects).insert(&object);
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
						for (auto& object : objects) {
							//if object has been consumed already, continue
							auto& consumedObjectLookup = std::get<ObjectLookup<Object>>(consumedObjects);
							if (consumedObjectLookup.contains(object)) {
								continue;
							}

							//mark object as consumed and then invoke function with it
							consumedObjectLookup.insert(object);
							func(*object);

							//if object is present in any other object groups, other objects in those may also need to be synced up
							unsyncedObjectGroups.push_range(object->groupIDs | std::views::filter([&, this](const auto& objectGroupID) {
								return  !syncedObjectGroups.contains(objectGroupID);
							}));
						}
						return nv::detail::STAY_IN_LOOP;
					}, group.objects);
				}
			}
		public:
			template<typename Object>
			void scale(EditedObjectData<Object>& object, float scale) {
				forEachImpl(object, &EditedObjectGroup::scaleSynced, [scale](auto& object) {
					object.obj.screenScale(scale);
					object.obj.worldScale(scale);
				});
			}
			void rotate(ID<EditedObjectGroup> id, float angle) {
				//forEachImpl(id, &EditedObjectGroup::rotationSynced, [angle](auto& object) {
				//	//object.obj.rotate(angle);
				//});
			}
			template<typename Object>
			void move(EditedObjectData<Object>& object, Point delta) {
				forEachImpl(object, &EditedObjectGroup::positionSynced, [delta](auto& object) {
					object.obj.screenMove(delta);
					object.obj.worldMove(delta);
				});
			}

			template<typename Object>
			void setOpacity(EditedObjectData<Object>& object, uint8_t opacity) {
				forEachImpl(object, &EditedObjectGroup::opacitySynced, [opacity](auto& object) {
					object.obj.setOpacity(opacity);
					object.opacity = opacity;
				});
			}
			decltype(auto) getGroup(this auto&& self, ID<EditedObjectGroup> id) {
				return self.m_objectGroups.at(id);
			}
		};
	}
}