#include "ObjectSearch.h"

namespace nv {
	namespace editor {
		class ObjectGroupCreator {
		private:
			std::string m_objectGroupName;
			ObjectSearch m_searchedObjects;
			UniformObjectVector m_currentlyStoredObjects;
		public:
			void setNewObjects(std::vector<Layer>& layers) {
				m_searchedObjects.setNewObjects(layers);
			}
		private:
			void showObjectSearchChild(NameManager& objectGroupNameManager) {
				ImGui::BeginChild("Object Search", { 700.0f, 700.0f });

				ImGui::SetNextItemWidth(getInputWidth());
				objectGroupNameManager.inputName("Object Group Name", m_objectGroupName);

				m_searchedObjects.show();
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::Button("Add Object")) {
					auto selectedObject = m_searchedObjects.getSelectedObject();
					if (selectedObject) {
						m_currentlyStoredObjects.objects.push_back(*selectedObject);
					}
				}
				ImGui::EndChild();
			}
			bool showCurrentStoredObjectsChild(ObjectGroupManager& objectGroupManager) {
				ImGui::SameLine();

				ImGui::BeginChild("Stored Objects", { 700.0f, 700.0f });
				for (auto [idx, objectVariant] : std::views::enumerate(m_currentlyStoredObjects.objects)) {
					//have option to remove object from the group
					ImGui::PushID(getUniqueImGuiID()); 
					if (ImGui::Button("X")) {
						m_currentlyStoredObjects.objects.erase(
							m_currentlyStoredObjects.objects.begin() + idx
						);
						ImGui::PopID();
						break;
					}
					ImGui::PopID();

					ImGui::SameLine();

					ImGui::PushID(getUniqueImGuiID());
					std::visit([&](auto& object) {
						ImGui::Text(object.get().getName().c_str());
					}, objectVariant);
					ImGui::PopID();
				}

				bool createdObjectGroup = false;

				if (ImGui::Button("Create Object Group")) {
					EditedObjectGroup newObjectGroup;
					newObjectGroup.name = m_objectGroupName;
					auto groupID = objectGroupManager.addGroup(std::move(newObjectGroup));
					for (auto& object : m_currentlyStoredObjects.objects) {
						std::visit([&](auto& objectRef) {
							objectRef.get().groupIDs.insert(groupID);
							newObjectGroup.addObject(&objectRef.get());
						}, object);
					}
					m_currentlyStoredObjects.clear();
					
					createdObjectGroup = true;
				}

				ImGui::EndChild();
				return createdObjectGroup;
			}
		public:
			bool show(ObjectGroupManager& objectGroupManager, NameManager& objectGroupNameManager) {
				bool showing = true;

				centerNextWindow();
				ImGui::SetNextWindowSize({ 1400.0f, 700.0f });
				ImGui::Begin(OBJECT_GROUP_CREATION_WINDOW_NAME, &showing, WINDOW_FLAGS);

				showObjectSearchChild(objectGroupNameManager);
				if (showCurrentStoredObjectsChild(objectGroupManager)) { //if we have created an object group, take down popup
					showing = false;
				}

				ImGui::End();

				return showing;
			}
		};
	}
}