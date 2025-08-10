#include "ObjectSearch.h"

void nv::editor::ObjectSearch::setNewObjects(std::vector<Layer>& layers) {
    m_objects.clear();
    for (auto& layer : layers) {
        nv::detail::forEachDataMember([this](auto& objectHive) {
            for (auto& object : objectHive) {
                m_objects.add(std::ref(object));
            }
            return nv::detail::STAY_IN_LOOP;
        }, layer.objects);
    }
    m_filteredObjects.addRange(m_objects.objects);
}

void nv::editor::ObjectSearch::appendFilter(size_t oldNameLen) {
    auto appendedText = m_currSearchedName.c_str() + oldNameLen;
    auto appendedTextLen = m_currSearchedName.size() - oldNameLen;
    m_filteredObjects.eraseIf([&, this](auto objectRef) {
        const auto& objectName = objectRef.get().getName();
        return objectName.size() < m_currSearchedName.size() ||
            std::string_view{ objectName }.substr(oldNameLen, appendedTextLen) != appendedText;
    });
}

void nv::editor::ObjectSearch::prependFilter() {
    m_filteredObjects.clear();
    m_objects.forEach([this](auto objectRef) {
        if (objectRef.get().getName().starts_with(m_currSearchedName)) {
            m_filteredObjects.add(objectRef);
        }
        return nv::detail::STAY_IN_LOOP;
    });
}

nv::editor::ObjectSearch::TextChange nv::editor::ObjectSearch::inputObjectName() {
    ImGui::SetNextItemWidth(getInputWidth());
    auto prevSearchedName = m_currSearchedName;
    if (ImGui::InputText("Object Name", &m_currSearchedName)) {
        if (m_currSearchedName.size() > prevSearchedName.size()) {
            if (m_currSearchedName.starts_with(prevSearchedName)) {
                return { prevSearchedName.size(), true, true };
            }
        } else {
            return { prevSearchedName.size(), true, false };
        }
    }
    return {};
}

void nv::editor::ObjectSearch::show() {
    auto [oldNameLen, wasTextChanged, wasTextAppended] = inputObjectName();
    if (wasTextChanged) {
        if (wasTextAppended) {
            appendFilter(oldNameLen);
        }
        else {
            prependFilter();
        }
    }

    int unnamedObjectCount = 0;

    m_filteredObjects.forEach([&, this](auto& objectRef) {
        bool selected = m_selectedObject.has_value() &&
            std::visit([](auto& object) { return object.get().id; }, *m_selectedObject) == objectRef.get().id;
        ImGui::PushID(getTemporaryImGuiID());
        if (ImGui::Selectable(objectRef.get().getName().c_str(), selected)) {
            m_selectedObject = std::make_optional<UniformObjectVector::ValueType>(objectRef);
        }
        ImGui::PopID();
        return nv::detail::STAY_IN_LOOP;
       });
}
