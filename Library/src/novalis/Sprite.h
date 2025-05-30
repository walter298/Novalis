#pragma once

#include "BufferedNode.h"
#include "DynamicNode.h"
#include "Event.h"

namespace nv {
    template<typename Node>
    void setCurrentLayer(Node& sprite, size_t l) {
        sprite.forEachExcept([](size_t layer, const auto& object) {
            object.setOpacity(0);
        }, l);
        sprite.forEach([](const auto& object) {
            object.setOpacity(255);
        }, l);
    }

    // template<typename Node>
    // Event<void> mapHorizontalMovement(Node& sprite, KeyState input, ) {
        
    // }
}