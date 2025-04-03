#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Boost::nowide" for configuration "Release"
set_property(TARGET Boost::nowide APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Boost::nowide PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/boost_nowide-vc143-mt-x64-1_86.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/boost_nowide-vc143-mt-x64-1_86.dll"
  )

list(APPEND _cmake_import_check_targets Boost::nowide )
list(APPEND _cmake_import_check_files_for_Boost::nowide "${_IMPORT_PREFIX}/lib/boost_nowide-vc143-mt-x64-1_86.lib" "${_IMPORT_PREFIX}/bin/boost_nowide-vc143-mt-x64-1_86.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
