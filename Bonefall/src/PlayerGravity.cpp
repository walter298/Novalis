#include "PlayerGravity.h"

#include <novalis/KeyState.h>
#include <novalis/physics/GravityManager.h>

struct PlayerGravity {
    static constexpr float STARTING_VELOCITY = 0.4f;

    float currVelocity = STARTING_VELOCITY;
    float acceleration = 1.008f;

    float getDownwardVelocity() noexcept {
        currVelocity *= acceleration;
        return currVelocity;
    }
    void reset() noexcept {
        currVelocity = STARTING_VELOCITY;
    }
};

static_assert(nv::physics::Gravity<PlayerGravity>);

using GravityManager = nv::physics::GravityManager<
    std::vector<std::reference_wrapper<const nv::BufferedPolygon>>,
    nv::BufferedNode, 
    nv::BufferedPolygon,
    PlayerGravity
>;

void setPlayerGravity(nv::EventHandler& handler, nv::BufferedNode& player, nv::BufferedPolygon& hitbox, 
    const nv::BufferedPolygon& ground) 
{
    std::vector groundPolygons{ std::ref(ground) };

	auto collisionManagerPtr = std::make_unique<GravityManager>(groundPolygons, player, hitbox);

	handler.add([playerGravityManager = collisionManagerPtr.get()]() mutable {
        constexpr auto speed = 1.53f;
        if (nv::EventHandler::isKeyPressed(nv::keys::D)) {
            playerGravityManager->move(speed);
        } else if (nv::EventHandler::isKeyPressed(nv::keys::A)) {
            playerGravityManager->move(-speed);
        }
	});

    handler.add([collisionManagerPtr = std::move(collisionManagerPtr)]() mutable {
		(*collisionManagerPtr)();
    });
}