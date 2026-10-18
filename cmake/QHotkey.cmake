# QHotkey 1.5.0 (Qt 6 support), pinned to its release commit. Keep its legacy
# CMake options and policy compatibility scoped to the dependency.
function(yh_add_qhotkey)
  set(QT_DEFAULT_MAJOR_VERSION 6)
  set(BUILD_SHARED_LIBS OFF)
  set(QHOTKEY_EXAMPLES OFF CACHE BOOL "Build QHotkey examples" FORCE)
  set(QHOTKEY_INSTALL OFF CACHE BOOL "Install QHotkey" FORCE)
  set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  FetchContent_Declare(qhotkey
    GIT_REPOSITORY https://github.com/Skycoder42/QHotkey.git
    GIT_TAG 34330d6ff5d2ca111c376f6d7da66be9d1817430
    EXCLUDE_FROM_ALL)
  FetchContent_MakeAvailable(qhotkey)
  if(APPLE)
    # The release uses shared Carbon hotkeys, which silently allow a second app
    # to register the same key, and never marks its event handlers installed.
    # Build a checked, minimally patched copy; leave downloaded sources intact.
    set(_mac_source "${qhotkey_SOURCE_DIR}/QHotkey/qhotkey_mac.cpp")
    file(SHA256 "${_mac_source}" _mac_hash)
    if(NOT _mac_hash STREQUAL "79dd5021113140a7b646aa2b878d9796566c8d2731ff95dcbe8d88e6fab087f8")
      message(FATAL_ERROR "Unexpected QHotkey macOS source; review the pinned compatibility patch")
    endif()
    file(READ "${_mac_source}" _mac_content)
    string(REGEX REPLACE "GetApplicationEventTarget\\(\\),[ \t\r\n]+0,"
      "GetApplicationEventTarget(),\n                                  kEventHotKeyExclusive," _mac_content "${_mac_content}")
    set(_install_release "InstallApplicationEventHandler(&QHotkeyPrivateMac::hotkeyReleaseEventHandler, 1, &releaseEventSpec, NULL, NULL);")
    string(REPLACE "${_install_release}" "${_install_release}\n        this->isHotkeyHandlerRegistered = true;"
      _mac_content "${_mac_content}")
    set(_patched "${PROJECT_BINARY_DIR}/qhotkey-macos/qhotkey_mac.cpp")
    file(GENERATE OUTPUT "${_patched}" CONTENT "${_mac_content}")
    set_source_files_properties("${_patched}" TARGET_DIRECTORY qhotkey PROPERTIES GENERATED TRUE)
    set_property(TARGET qhotkey PROPERTY SOURCES "${qhotkey_SOURCE_DIR}/QHotkey/qhotkey.cpp" "${_patched}")
    target_include_directories(qhotkey PRIVATE "${qhotkey_SOURCE_DIR}/QHotkey")
  endif()
  install(FILES "${qhotkey_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/YandexHome/licenses/QHotkey")
endfunction()
