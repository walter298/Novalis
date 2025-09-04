#include "Player.h"

#include <novalis/Camera.h>
#include <novalis/Instance.h>
#include <novalis/physics/GravityManager.h>

#include "Gravity.h"

using NodeRef = std::reference_wrapper<nv::BufferedNode>;
using PolyRef = std::reference_wrapper<nv::BufferedPolygon>;
using SpritesheetRef = std::reference_wrapper<nv::Spritesheet>;

struct CameraMonoid {
    NodeRef root;
    NodeRef player;
    nv::CameraLock lock;

    void move(nv::Point delta) noexcept {
        lock.move(root.get(), player.get(), delta);
    }
};

static CameraMonoid makePlayerMover(nv::BufferedNode& root, nv::BufferedNode& player, const nv::BufferedPolygon& playerHitbox) {
    //get the background frame
    const auto& background = root.find<nv::Texture>("background");
    nv::Rect backgroundFrame{ background.getScreenPos(), background.getScreenSize() };

    //make camera follow the center of the player
    auto playerCentroid = playerHitbox.calcScreenCentroid();

    nv::CameraLock cameraLock{ backgroundFrame, playerHitbox.calcScreenCentroid() };
    return CameraMonoid{ root, player, std::move(cameraLock) };
}

using GravityManager = nv::physics::GravityManager<
    const std::span<nv::BufferedPolygon>, CameraMonoid, nv::BufferedPolygon, rm::Gravity
>;

class PlayerMovement {
private:
    static constexpr auto STANDING_INDEX = 15;

    GravityManager m_gravityManager;
    SpritesheetRef m_spritesheet;
    bool m_facingRight = true;

    void changeRunningTexture() {
        auto nextTextureIndex = ((m_spritesheet.get().getColumnIndex() + 1) % STANDING_INDEX);
        if (nextTextureIndex == 0) {
            nextTextureIndex++;
        }
        m_spritesheet.get().setTextureIndex(0, nextTextureIndex);
    }

    void faceDirectionOfMovement(float dx) {
        constexpr auto HITBOX_SHIFT = 97.0f;

        auto& player = m_gravityManager.getObject().player.get();
        auto& hitbox = m_gravityManager.getHitbox();
        if (dx > 0.0f && !m_facingRight) {
            m_facingRight = true;
            player.flipHorizontally();
            hitbox.move({ HITBOX_SHIFT, 0.0f });
        } else if (dx < 0.0f && m_facingRight) {
            m_facingRight = false;
            player.flipHorizontally();
            hitbox.move({ -HITBOX_SHIFT, 0.0f });
        }
    }
public:
    PlayerMovement(nv::BufferedNode& root, CameraMonoid playerMover, nv::BufferedPolygon& hitbox) :
        m_gravityManager{ root.findObjectsInLayer<nv::BufferedPolygon>("ground_polygons"), playerMover, hitbox
    }, m_spritesheet{ playerMover.player.get().find<nv::Spritesheet>("spritesheet") }
    {
    }

    void operator()() {
        constexpr auto DX = 34.0f;

        auto moveImpl = [this](float dx) {
            m_gravityManager.move(dx);
            faceDirectionOfMovement(dx);
            changeRunningTexture();
        };

        if (nv::EventHandler::isKeyPressed(nv::keys::A)) {
            moveImpl(-DX);
        } else if (nv::EventHandler::isKeyPressed(nv::keys::D)) {
            moveImpl(DX);
        } else {
            m_spritesheet.get().setTextureIndex(0, STANDING_INDEX);
        }
        m_gravityManager();
    }
};

void rm::player::setPlayerMovement(nv::EventHandler& evtHandler, nv::BufferedNode& root) {
    auto& player = root.find<nv::BufferedNode>("player");
    auto& playerHitbox = player.find<nv::BufferedPolygon>("hitbox");
    auto playerMover = makePlayerMover(root, player, playerHitbox);
    playerHitbox.setOpacity(255);

    using namespace std::literals;
    evtHandler.add(PlayerMovement{ root, std::move(playerMover), playerHitbox }, 30ms);
}
