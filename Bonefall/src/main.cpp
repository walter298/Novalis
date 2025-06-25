#include <print>
#include <filesystem>

#include <novalis/Instance.h>
#include <novalis/Scene.h>
#include <novalis/detail/file/File.h>
#include <novalis/detail/serialization/BufferedNodeSerialization.h>

#include "Bone.h"
#include "PlayerGravity.h"

std::string workingDirectory() {
	auto path = std::filesystem::current_path().parent_path().parent_path();
	return path.string();
}

int main() {
    nv::Instance instance{ "Bonefall" };

    //load bonefall node
    auto bonefallNodePath = workingDirectory() + "/assets/nodes/bonefall.nv_node";
    auto root = instance.registry.loadBufferedNode(bonefallNodePath);

    //look up player data
    auto& player = root.find<nv::BufferedNode>("skull");
    auto& playerHitbox = player.find<nv::BufferedPolygon>("hitbox");
    playerHitbox.setOpacity(0);

    nv::EventHandler handler;

    //set up player gravity and collision
    auto& ground = root.find<nv::BufferedPolygon>("ground_");
    ground.setOpacity(255);
    setPlayerGravity(handler, player, playerHitbox, ground);

    //set up bone falling
    auto& bone = root.find<nv::BufferedNode>("bone");
    handleBoneFalling(handler, bone, playerHitbox);

    using namespace std::literals;
    auto framerate = 1s / 60;

    nv::showScene(root, instance.renderer, handler, framerate);
}