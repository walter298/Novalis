#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "nfd::nfd" for configuration "Release"
set_property(TARGET nfd::nfd APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(nfd::nfd PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/nfd.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/nfd.dll"
  )

list(APPEND _cmake_import_check_targets nfd::nfd )
list(APPEND _cmake_import_check_files_for_nfd::nfd "${_IMPORT_PREFIX}/lib/nfd.lib" "${_IMPORT_PREFIX}/bin/nfd.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
