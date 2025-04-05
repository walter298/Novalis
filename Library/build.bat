cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/package_managers/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake -G "Visual Studio 17 2022" -B build -DCMAKE_BUILD_TYPE=Release
