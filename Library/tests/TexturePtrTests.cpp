#include <algorithm>
#include <vector>

#include "TexturePtrTests.h"
#include "novalis/detail/memory/TexturePtr.h"
#include "novalis/Instance.h"

constexpr const char* IMAGE_PATH = "C:/Users/walte/OneDrive/Desktop/Code_Libraries/Novalis_CMake_Build/Redmane/assets/images/orc/spritesheet.png";

using nv::detail::TexturePtr;

static void testRefCount(TexturePtr& tex) {
	//copying
	for (size_t texCount = 15; texCount < 30; texCount++) {
		assert(tex.tex->refcount == 2);
		{
			std::vector<TexturePtr> vector(texCount);
			std::ranges::fill(vector, tex);
			assert(tex.tex->refcount == texCount + 2);
		}
	}

	//moving
	auto texOwner = std::move(tex);
	assert(texOwner.tex->refcount == 2);
	tex = std::move(texOwner);
	assert(tex.tex->refcount == 2);
}

void runTexturePtrTests(nv::Instance& instance) {
	try {
		auto tex = instance.registry.loadTexture(instance.getRenderer(), IMAGE_PATH);

		std::println("Testing refcount...");
		testRefCount(tex);
		std::println("Refcount test passed :)");
	} catch (const std::exception& e) {
		std::println("Exception thrown during TexturePtr tests: {}", e.what());
	}
}
