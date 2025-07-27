#include "ObjectGroupManager.h"

nv::ID<nv::editor::EditedObjectGroup> nv::editor::ObjectGroupManager::addGroupImpl(std::string&& name) {
	auto [it, inserted] = m_objectGroups.emplace(
		std::piecewise_construct, std::forward_as_tuple(), std::forward_as_tuple()
	);
	m_currUniversalObjectGroupID = it->first;
	it->second.name = std::move(name);
	return it->first;
}

nv::ID<nv::editor::EditedObjectGroup> nv::editor::ObjectGroupManager::addGroup(std::string name) {
	m_objectGroupNameManager.makeNewName(name);
	return addGroupImpl(std::move(name));
}

nv::ID<nv::editor::EditedObjectGroup> nv::editor::ObjectGroupManager::addGroup() {
	return addGroupImpl(m_objectGroupNameManager.makeUniqueName());
}

void nv::editor::ObjectGroupManager::editObjectGroupName(ID<EditedObjectGroup> id) {
	auto& objectGroup = m_objectGroups.at(id);
	m_objectGroupNameManager.inputName("Object group name", objectGroup.name);
}

void nv::editor::ObjectGroupManager::removeGroup(ID<EditedObjectGroup> id) {
	nv::detail::forEachDataMember([&](auto& objects) {
		for (auto& object : objects) {
			object->groupIDs.erase(id);
		}
		return nv::detail::STAY_IN_LOOP;
	}, m_objectGroups.at(id).objects);
	m_objectGroups.erase(id);
}

void nv::editor::ObjectGroupManager::showObjectGroupOptions(EditedObjectGroup& objectGroup) {
	auto showSyncOption = [](const char* label, EditedObjectGroup::SyncOption& syncOption) {
		bool temp = syncOption;
		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Checkbox(label, &temp)) {
			syncOption = static_cast<EditedObjectGroup::SyncOption>(temp);
		}
	};
	showSyncOption("Sync position", objectGroup.positionSynced);
	showSyncOption("Sync rotation", objectGroup.rotationSynced);
	showSyncOption("Sync scale", objectGroup.scaleSynced);
	showSyncOption("Sync opacity", objectGroup.opacitySynced);
}

void nv::editor::ObjectGroupManager::showDeletionOption() {
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::Button("Delete group")) {
		auto& deletedObjectGroup = m_objectGroups.at(m_currUniversalObjectGroupID);
		nv::detail::forEachDataMember([this](auto& objects) {
			objects.clear();
			return nv::detail::STAY_IN_LOOP;
		}, deletedObjectGroup.objects);
		m_objectGroups.erase(m_currUniversalObjectGroupID);
		m_currUniversalObjectGroupID = ID<EditedObjectGroup>::None();
	}
}

void nv::editor::ObjectGroupManager::showCurrentObjectGroupOptions(NameManager& objectGroupNameManager) {
	if (m_currUniversalObjectGroupID != ID<EditedObjectGroup>::None()) {
		auto& currObjectGroup = m_objectGroups.at(m_currUniversalObjectGroupID);
		objectGroupNameManager.inputName("Group name", currObjectGroup.name);
		showObjectGroupOptions(currObjectGroup);
	}
}

void nv::editor::ObjectGroupManager::showObjectGroupDropdownForAllObjectGroups() {
	auto previewObjectGroupName = m_currUniversalObjectGroupID == ID<EditedObjectGroup>::None() ?
		"No Group" : m_objectGroups.at(m_currUniversalObjectGroupID).name.c_str();

	if (ImGui::BeginCombo("Object group", previewObjectGroupName)) {
		for (const auto& [objectGroupID, objectGroup] : m_objectGroups) {
			ImGui::SetNextItemWidth(getInputWidth());
			ImGui::PushID(getTemporaryImGuiID());
			if (ImGui::Selectable(objectGroup.name.c_str(), objectGroupID == m_currUniversalObjectGroupID)) {
				m_currUniversalObjectGroupID = objectGroupID;
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
}

void nv::editor::ObjectGroupManager::showObjectGroupCreationButton(bool& creatingObjectGroup) {
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::Button("New Object Group")) {
		creatingObjectGroup = true;
	}
}
