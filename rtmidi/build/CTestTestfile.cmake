# CMake generated Testfile for 
# Source directory: D:/nvidia/volume/VAAPI/rtmidi
# Build directory: D:/nvidia/volume/VAAPI/rtmidi/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(apinames "D:/nvidia/volume/VAAPI/rtmidi/build/tests/Debug/apinames.exe")
  set_tests_properties(apinames PROPERTIES  _BACKTRACE_TRIPLES "D:/nvidia/volume/VAAPI/rtmidi/CMakeLists.txt;213;add_test;D:/nvidia/volume/VAAPI/rtmidi/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(apinames "D:/nvidia/volume/VAAPI/rtmidi/build/tests/Release/apinames.exe")
  set_tests_properties(apinames PROPERTIES  _BACKTRACE_TRIPLES "D:/nvidia/volume/VAAPI/rtmidi/CMakeLists.txt;213;add_test;D:/nvidia/volume/VAAPI/rtmidi/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(apinames "D:/nvidia/volume/VAAPI/rtmidi/build/tests/MinSizeRel/apinames.exe")
  set_tests_properties(apinames PROPERTIES  _BACKTRACE_TRIPLES "D:/nvidia/volume/VAAPI/rtmidi/CMakeLists.txt;213;add_test;D:/nvidia/volume/VAAPI/rtmidi/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(apinames "D:/nvidia/volume/VAAPI/rtmidi/build/tests/RelWithDebInfo/apinames.exe")
  set_tests_properties(apinames PROPERTIES  _BACKTRACE_TRIPLES "D:/nvidia/volume/VAAPI/rtmidi/CMakeLists.txt;213;add_test;D:/nvidia/volume/VAAPI/rtmidi/CMakeLists.txt;0;")
else()
  add_test(apinames NOT_AVAILABLE)
endif()
