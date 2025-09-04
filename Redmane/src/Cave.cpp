#include "Cave.h"

void setupCave() {

}

nv::BufferedNode rm::makeCave(nv::ResourceRegistry& registry, const std::string& workingDirectory) {
    auto cave = registry.loadBufferedNode(workingDirectory + "/assets/nodes/cave.nv_node");
    cave.forEachExcept([](auto layer, auto& object) {
        object.setOpacity(255);
        return nv::detail::STAY_IN_LOOP;
    }, "ground_polygons");
    cave.forEach([](auto layer, auto& object) {
        object.setOpacity(255);
        return nv::detail::STAY_IN_LOOP;
    }, "ground_polygons");
    
    auto& background = cave.find<nv::Texture>("background");
    auto delta = background.getScreenPos();
    cave.move(-delta);

    return cave;
}
