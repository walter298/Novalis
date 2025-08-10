#pragma once

#include <algorithm>
#include <ranges>
#include <variant>
#include <vector>

#include <novalis/detail/reflection/ClassIteration.h>
#include "EditedObjectData.h"

namespace nv {
    namespace editor {
        template<typename... Ts>
        struct Index {
            using ValueType = std::variant<std::reference_wrapper<Ts>...>;
        
            std::vector<ValueType> objects;
        
            template<typename T>
            void add(std::reference_wrapper<T> ref) {
                objects.push_back(ValueType{ ref });
            }

            template<std::ranges::viewable_range Range>
            void addRange(Range&& range) {
                objects.append_range(std::forward<Range>(range));
            }

            template<typename Pred>
            void sort(Pred pred) {
                std::ranges::stable_sort(objects, [&](const auto& a, const auto& b) {
                    return std::visit([&](const auto& v1, const auto& v2) {
                        return pred(a, b);
                    }, a, b);
                });
            }

            template<typename Func>
            void forEach(this auto&& self, Func f) {
                for (auto& variant : self.objects) {
                    if (std::visit(f, variant) == nv::detail::BREAK_FROM_LOOP) {
                        break;
                    }
                }
            }

            template<typename Pred>
            void eraseIf(Pred f) {
                std::erase_if(objects, [&](const auto& variant) {
                    return std::visit(f, variant);
                }); 
            }

            void clear() {
                objects.clear();
            }
        };

        using UniformObjectVector = Index<
            ObjectMetadata<BufferedNode>,
            ObjectMetadata<DynamicPolygon>,
            ObjectMetadata<Texture>,
            ObjectMetadata<Spritesheet>
        >;
    }
}