#pragma once

#include <string>
#include <vector>
#include <boost/unordered/unordered_flat_map.hpp>
#include <plf_hive.h>

#include <novalis/BufferedNode.h>
#include <novalis/Polygon.h>

#include "imgui/ImGui.h"
#include "imgui/imgui_stdlib.h"

#include "ImGuiID.h"
#include "Layer.h"
#include "ObjectIndex.h"
#include "WindowLayout.h"

namespace nv {
    namespace editor {
        class ObjectSearch {
        private:
            template<typename Object>
            static consteval size_t getTypeRanking() {
                if constexpr (std::same_as<Object, BufferedNode>) {
                    return 0;
                } else if constexpr (std::same_as<Object, Texture>) {
                    return 1;
                } else {
                    return 2;
                }
            }

            UniformObjectVector m_objects;
            UniformObjectVector m_filteredObjects;
            std::optional<UniformObjectVector::ValueType> m_selectedObject;
            
            std::string m_currSearchedName;

            template<typename Object>
            using SelectionMap = boost::unordered_flat_map<ID<Object>, bool>;
            using SelectedObjects = std::tuple<SelectionMap<BufferedNode>, SelectionMap<DynamicPolygon>, SelectionMap<Texture>>;
            
            SelectedObjects m_checkedObjects;
        public:
            void setNewObjects(std::vector<Layer>& layers) {
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
        
            std::optional<UniformObjectVector::ValueType> getSelectedObject() {
                return m_selectedObject;
            }
        private:
            void appendFilter(size_t oldNameLen) {
                auto appendedText = m_currSearchedName.c_str() + oldNameLen;
                auto appendedTextLen = m_currSearchedName.size() - oldNameLen;
                m_filteredObjects.eraseIf([&, this](auto objectRef) {
                    const auto& objectName = objectRef.get().getName();
                    return objectName.size() < m_currSearchedName.size() ||
                           std::string_view{ objectName }.substr(oldNameLen, appendedTextLen) != appendedText;
                });
            }

            void prependFilter() {
                m_filteredObjects.clear();
                m_objects.forEach([this](auto objectRef) {
                    if (objectRef.get().getName().starts_with(m_currSearchedName)) {
                        m_filteredObjects.add(objectRef);
                    }
                    return nv::detail::STAY_IN_LOOP;
                });
            }

            struct TextChange {
                size_t oldTextLen = 0; //only relevant if text was appended
                bool isChanged = false;
                bool isAppended = false;
            };

            TextChange inputObjectName() {
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
        public:
            void show() {
                auto [oldNameLen, wasTextChanged, wasTextAppended] = inputObjectName();
                if (wasTextChanged) {
                    if (wasTextAppended) {
                        appendFilter(oldNameLen);
                    } else {
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
        };
    }
}