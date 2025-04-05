vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO walter298/Novalis
    REF "${VERSION}"
    SHA512 40565c5e65fcb273ed081cadaafee47769eef5e95c70732ab6a155d1c2fa38222f77fe2ac955f30afa8fb13130056de7719436e271fc38e039aff6caf9797e00 
    HEAD_REF cmake-sample-lib
)

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}/Library")

vcpkg_cmake_build()
vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME "Novalis")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)