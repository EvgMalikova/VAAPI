#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "RtMidi::rtmidi" for configuration "Release"
set_property(TARGET RtMidi::rtmidi APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(RtMidi::rtmidi PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/rtmidi.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/rtmidi.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS RtMidi::rtmidi )
list(APPEND _IMPORT_CHECK_FILES_FOR_RtMidi::rtmidi "${_IMPORT_PREFIX}/lib/rtmidi.lib" "${_IMPORT_PREFIX}/bin/rtmidi.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
