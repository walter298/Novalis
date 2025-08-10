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
            void setNewObjects(std::vector<Layer>& layers);
        
            std::optional<UniformObjectVector::ValueType> getSelectedObject() {
                return m_selectedObject;
            }
        private:
            void appendFilter(size_t oldNameLen);
            void prependFilter();

            struct TextChange {
                size_t oldTextLen = 0; //only relevant if text was appended
                bool isChanged = false;
                bool isAppended = false;
            };

            TextChange inputObjectName();
        public:
            void show();
        };
    }
}