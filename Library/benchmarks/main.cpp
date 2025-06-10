#include "Benchmark.h"

#include "../src/novalis/BufferedNode.h"
#include "../src/novalis/DynamicNode.h"
#include "../src/novalis/Instance.h"
#include "../src/novalis/NodeFileVerification.h"

int main() {
    std::println("Running newest build...");

    nv::Instance instance{ "Benchmark" };

    std::println("Loading test node...");

    constexpr auto path = "C:/Users/walte/OneDrive/Desktop/Code_Libraries/Novalis_CMake_Build/Redmane/assets/nodes/player_v2.nv_node";
    
    auto dynamicNode = instance.registry.loadDynamicNode(path);
    auto bufferedNode = instance.registry.loadBufferedNode(path);

    auto render = [&](auto& node) {
        SDL_RenderClear(instance.renderer);
        node.render(instance.renderer);
        SDL_RenderClear(instance.renderer);
    };

    std::println("Averaging render time...");

    constexpr int ITERATIONS = 100;
    using std::chrono::microseconds;
    std::println("Average dynamic node render time: {}", average<microseconds>([&]() { render(dynamicNode); }, ITERATIONS));
    std::println("Average buffered node render time: {}", average<microseconds>([&]() { render(bufferedNode); }, ITERATIONS));
}