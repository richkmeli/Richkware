#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Richkware::richkware" for configuration ""
set_property(TARGET Richkware::richkware APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(Richkware::richkware PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/richkware"
  )

list(APPEND _cmake_import_check_targets Richkware::richkware )
list(APPEND _cmake_import_check_files_for_Richkware::richkware "${_IMPORT_PREFIX}/bin/richkware" )

# Import target "Richkware::richkware_lib" for configuration ""
set_property(TARGET Richkware::richkware_lib APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(Richkware::richkware_lib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/librichkware_lib.a"
  )

list(APPEND _cmake_import_check_targets Richkware::richkware_lib )
list(APPEND _cmake_import_check_files_for_Richkware::richkware_lib "${_IMPORT_PREFIX}/lib/librichkware_lib.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
