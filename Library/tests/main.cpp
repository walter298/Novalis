#include "BufferedNodeTests.h"
#include "TexturePtrTests.h"

int main() {
	nv::Instance instance{ "Tests" };
	runTexturePtrTests(instance);
	runBufferedNodeTests();
}