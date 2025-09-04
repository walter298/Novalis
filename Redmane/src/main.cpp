#include <filesystem>

#include <novalis/Instance.h>
#include <novalis/Scene.h>
#include <novalis/detail/serialization/BufferedNodeSerialization.h>

#include "Cave.h"
#include "Knight.h"
#include "Player.h"

auto workingDirectory() {
    return std::filesystem::current_path().parent_path().parent_path().string();
}

int main() {
    nv::Instance instance{ "Redmane" };

    auto cave = rm::makeCave(instance.registry, workingDirectory());
    
    nv::EventHandler evtHandler;
    rm::player::setPlayerMovement(evtHandler, cave);

	rm::makeKnightAI(evtHandler, cave);

    using namespace std::literals;
    nv::showScene(cave, instance.getRenderer(), evtHandler, 1s / 60);
}