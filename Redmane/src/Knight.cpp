#include <novalis/physics/GravityManager.h>

#include "Gravity.h"
#include "Knight.h"

using NodeRef = std::reference_wrapper<nv::BufferedNode>;
using PolyRef = std::reference_wrapper<nv::BufferedPolygon>;
using SpritesheetRef = std::reference_wrapper<nv::Spritesheet>;

using GravityManager = nv::physics::GravityManager<
	std::span<nv::BufferedPolygon>, NodeRef,
	nv::BufferedPolygon, rm::Gravity
>;

class Knight {
public:
	static inline nv::BufferedPolygon* playerHitbox = nullptr;
private:
	static constexpr int RUNNING_BEGIN_INDEX = 0;
	static constexpr int RUNNING_END_INDEX = 4;
	//static constexpr int STABBING_INDEX = ;
	static constexpr int STANDING_INDEX = 6;

	GravityManager m_gravityManager;
	SpritesheetRef m_spritesheet;
	//PolyRef m_sword;
	int m_health = 7;
	bool m_facingRight = false;
	bool m_inAttackAnimation = false;

	/*bool inAttackRange() const noexcept {
		return nv::physics::intersects(m_sword.get(), *playerHitbox);
	}*/

	void flipKnightToFacePlayer(float deltaX) {
		if (deltaX < 0.0f && m_facingRight) {
			m_facingRight = false;
			m_gravityManager.getObject().flipHorizontally();
		} else if (deltaX > 0.0f && !m_facingRight) {
			m_facingRight = true;
			m_gravityManager.getObject().flipHorizontally();
		}
	}

	/*void startAttackAnimation(nv::EventHandler& evtHandler) {
		m_inAttackAnimation = true;
		m_spritesheet.get().setTextureIndex(0, STANDING_INDEX);

		using namespace std::literals;
		evtHandler.chain(
			1s,
			[this] {
				m_spritesheet.get().setTextureIndex(0, STABBING_INDEX);
				return nv::EventHandler::END_EVENT;
			},
			[this] {
				m_spritesheet.get().setTextureIndex(0, STANDING_INDEX);
				m_inAttackAnimation = false;
				return nv::EventHandler::END_EVENT;
			}
		);
	}*/

	void runTowardsPlayer(nv::Point path) noexcept {
		//do running animation
		auto currColIdx = m_spritesheet.get().getColumnIndex();
		auto colCount = m_spritesheet.get().getColumnCount();
		auto newColIdx = (currColIdx + 1) % colCount;
		if (newColIdx == 0) {
			newColIdx = 2;
		}
		m_spritesheet.get().setTextureIndex(0, newColIdx);
		
		//move toward the player
		constexpr float DELTA_X = 20.0f;
		nv::Point delta{ std::copysignf(DELTA_X, path.x), 0.0f };
		m_gravityManager.move(delta.x);
		m_gravityManager();
	}
public:
	Knight(nv::BufferedNode& node, nv::BufferedPolygon& hitbox, std::span<nv::BufferedPolygon> groundPolygons) 
		: m_gravityManager{ groundPolygons, node, hitbox }, m_spritesheet{ node.find<nv::Spritesheet>("spritesheet") }
		//m_sword{ node.find<nv::BufferedPolygon>("sword") }
	{
		m_spritesheet.get().setTextureIndex(0, STANDING_INDEX);
		//m_sword.get().setOpacity(0);
		m_gravityManager.getHitbox().setOpacity(0);
	}

	void attack(nv::EventHandler& evtHandler) {
		if (m_inAttackAnimation) {
			m_gravityManager();
			return;
		}

		//auto swordCentroid = m_sword.get().calcWorldCentroid();
		auto pathToPlayer = playerHitbox->getWorldPos() - m_gravityManager.getHitbox().calcWorldCentroid();

		flipKnightToFacePlayer(pathToPlayer.x);

		runTowardsPlayer(pathToPlayer);

		//if we are in the attack range, attack!
		//if (nv::physics::intersects(m_sword.get(), *playerHitbox)) {
		//	startAttackAnimation(evtHandler);
		//} else { //otherwise, run toward the player
		//	runTowardsPlayer(pathToPlayer);
		//}
	}
};

void rm::makeKnightAI(nv::EventHandler& evtHandler, nv::BufferedNode& root) {
	using namespace std::literals;

	Knight::playerHitbox = &root.find<nv::BufferedNode>("player").find<nv::BufferedPolygon>("hitbox");
	auto& polygons = root.findObjectsInLayer<nv::BufferedPolygon>("ground_polygons");

	auto& knightNodes = std::get<std::span<nv::BufferedNode*>>(root.findObjectGroup("knights"));
	for (auto knight : knightNodes) {
		auto& knightHitbox = knight->find<nv::BufferedPolygon>("hitbox");
		evtHandler.add([&evtHandler, knight = Knight{ std::ref(*knight), knightHitbox, polygons }]() mutable {
			knight.attack(evtHandler);
		}, 40ms);
	}
}
