#pragma once

#include "ImGuiID.h"
#include "NameManager.h"
#include "ObjectGroup.h"
#include "WindowLayout.h"

namespace nv {
	namespace editor {
		class ObjectGroupManager {
		private:
			NameManager m_objectGroupNameManager{ "Object Group " };
			boost::unordered_flat_map<ID<EditedObjectGroup>, EditedObjectGroup> m_objectGroups;
			ID<EditedObjectGroup> m_currUniversalObjectGroupID        = ID<EditedObjectGroup>::None();
			ID<EditedObjectGroup> m_currObjectSpecificObjectGroupID = ID<EditedObjectGroup>::None();

			ID<EditedObjectGroup> addGroupImpl(std::string&& name);
		public:
			ID<EditedObjectGroup> addGroup(std::string name);
			ID<EditedObjectGroup> addGroup();
			void editObjectGroupName(ID<EditedObjectGroup> id);

			template<typename Object>
			void addObjectToGroup(ID<EditedObjectGroup>& groupID, EditedObjectData<Object>& object) {
				object.groupIDs.insert(groupID);
				m_objectGroups.at(groupID).addObject(&object);
			}

			void removeGroup(ID<EditedObjectGroup> id);

			template<typename Object>
			void removeFromAllObjectGroups(EditedObjectData<Object>& object) {
				for (const auto& groupID : object.groupIDs) {
					auto& group = m_objectGroups.at(groupID);
					group.removeObject(&object);
				}
				object.groupIDs.clear();
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
			void rotate(ID<EditedObjectGroup> id, float angle) { //todo: support rotation
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

			const auto& getAllGroups() const noexcept {
				return m_objectGroups;
			}
		private:
			void showObjectGroupOptions(EditedObjectGroup& objectGroup);
			void showDeletionOption();
			void showCurrentObjectGroupOptions(NameManager& objectGroupNameManager);
			void showObjectGroupDropdownForAllObjectGroups();
		public:
			void showAllObjectGroups(NameManager& objectGroupNameManager) {
				showObjectGroupDropdownForAllObjectGroups();
				showCurrentObjectGroupOptions(objectGroupNameManager);
			}
		private:
			template<typename Object>
			void showObjectGroupDropdownForObject(EditedObjectData<Object>& object) {
				auto previewObjectGroupName = m_currObjectSpecificObjectGroupID == ID<EditedObjectGroup>::None() ?
					"No Group" : m_objectGroups.at(m_currObjectSpecificObjectGroupID).name.c_str();

				if (ImGui::BeginCombo("Object group", previewObjectGroupName)) {
					for (const auto& objectGroupID : object.groupIDs) {
						ImGui::PushID(getTemporaryImGuiID());
						if (ImGui::Button("X")) {
							object.groupIDs.erase(objectGroupID);
							auto& objectGroup = m_objectGroups.at(objectGroupID);
							std::get<ObjectLookup<Object>>(objectGroup.objects).erase(&object);
							ImGui::PopID();
							break; //prevent iterator invalidation
						}
						ImGui::PopID();
						ImGui::SameLine();
						auto& objectGroup = m_objectGroups.at(objectGroupID);
						ImGui::SetNextItemWidth(getInputWidth());
						ImGui::PushID(getTemporaryImGuiID());
						if (ImGui::Selectable(objectGroup.name.c_str(), objectGroupID == m_currObjectSpecificObjectGroupID)) {
							m_currUniversalObjectGroupID = objectGroupID;
							m_currObjectSpecificObjectGroupID = objectGroupID;
						}
						ImGui::PopID();
					}
					ImGui::EndCombo();
				}
			}

			void showObjectGroupCreationButton(bool& creatingObjectGroup);
		public:
			template<nv::concepts::RenderableObject Object>
			void showObjectGroupsOfObject(EditedObjectData<Object>& object, bool& creatingObjectGroup) {
				showObjectGroupDropdownForObject(object);
				showObjectGroupCreationButton(creatingObjectGroup);
			}
		};
	}
}