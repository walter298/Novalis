#include "BufferedNodeTests.h"
#include "novalis/Instance.h"

static void runTextureNullityTest(nv::BufferedNode& node) {
	node.forEach([]<typename Object>(auto layer, Object& object) {
		if constexpr (std::same_as<Object, nv::BufferedNode>) {
			runTextureNullityTest(object);
		} else {
			nv::detail::forEachDataMember([]<typename Member>(Member & member) {
				if constexpr (std::same_as<Member, nv::detail::TexturePtr>) {
					assert(member.tex);
				} 
				return nv::detail::STAY_IN_LOOP;
			}, object);
		}
		
		return nv::detail::STAY_IN_LOOP;
	});
}

void runBufferedNodeTests() {
	std::println("Running node tests...");
	constexpr auto path = "C:/Users/walte/OneDrive/Desktop/Code_Libraries/Novalis_CMake_Build/Redmane/assets/nodes/cave.nv_node";
	
	//load in test node
	auto& registry = nv::getGlobalInstance()->registry;
	auto node = registry.loadBufferedNode(path);

	std::println("Running texture nullity test...");
	runTextureNullityTest(node);
	auto copy = node;
	runTextureNullityTest(copy);
	auto movedFrom = std::move(copy);
	runTextureNullityTest(movedFrom);
	std::println("Texture nullity test passed!");

	std::println("All node tests succeeded!");
}
