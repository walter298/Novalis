vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO walter298/Novalis
    REF "${VERSION}"
    SHA512 1f0d7c72aa1bc672a37e75f1975f660fd0da729ce356a0e1cd289f1c3f520a375d6f333877187743b07126bd0d162819d35f4ed31068c390cc2d4d12200b07e2
    HEAD_REF cmake-sample-lib
)

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}/Library")

vcpkg_cmake_build()
vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME "Novalis")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)