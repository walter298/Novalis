#include "App.h"
#include "tests/NodeFileTests.h"

int main() {
    nv::editor::runApp();
    /*try {
        nv::Instance instance{ "Foobar" };

        namespace fs = std::filesystem;
       
        nv::editor::testing::testNodeFileValidity(fs::directory_iterator{
            "C:/Users/walte/OneDrive/Desktop/Code_Libraries/Novalis_CMake_Build/Redmane/assets/nodes"
        });
    } catch (std::exception& e) {
        std::println(stderr, "{}", e.what());
    }*/
}