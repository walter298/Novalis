#include "Bone.h"

#include <novalis/physics/Intersection.h>
#include <novalis/physics/Jump.h>
#include <novalis/physics/Path.h>
#include <novalis/Scene.h>

constexpr auto BONE_BOUNCE_VELOCITY = 4.0f;
constexpr auto BONE_FALL_VELOCITY   = BONE_BOUNCE_VELOCITY;

struct Bonefall {
	float velocity = BONE_FALL_VELOCITY;
	std::optional<nv::Point> move() noexcept {
		constexpr auto ACCELERATION = 1.01f;
		velocity *= ACCELERATION;
		return nv::Point{ 0.0f, velocity };
	}
};

static constexpr auto LOW_THRESHOLD = -1000.0f;

void handleBoneFalling(nv::EventHandler& evtHandler, nv::BufferedNode& bone, const nv::BufferedPolygon& playerHitbox) 
{
	auto boneHitbox = std::ref(bone.find<nv::BufferedPolygon>("hitbox"));
	boneHitbox.get().setOpacity(0);
	bone.move({ 0.0f, -1000.0f });

	evtHandler.add([&, currVelocity = 0.001f, acceleration = 1.01f]() mutable {
		//gravitate toward the player
		const auto& [px, py] = playerHitbox.getWorldPoints()[0];
		const auto& [bx, by] = boneHitbox.get().getWorldPoints()[0];
		bone.move({ px - bx, currVelocity });
		currVelocity *= acceleration;

		//do little mario jump animation if we hit the player
		if (nv::physics::intersects(boneHitbox.get(), playerHitbox)) {
			nv::physics::Jump jump{ 300.0f, 4.0f };
			evtHandler.chain(
				nv::physics::makePath(bone, jump), 
				nv::physics::makePath(bone, Bonefall{})
			);
			return nv::EventHandler::END_EVENT;
		} 

		return nv::EventHandler::CONTINUE_EVENT;
	});
}
