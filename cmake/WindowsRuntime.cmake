# Keep build-tree launches independent of other Qt/MinGW installations on PATH.
# This runs before CTest, whereas install-time deployment runs after the tests.
if(NOT WIN32)
  return()
endif()

set(_runtime_dlls)
set(_runtime_targets)
foreach(_target appYandexHomeDesktop YandexHomeCli YandexHomeRest YandexHomeShortcuts)
  if(TARGET ${_target})
    list(APPEND _runtime_targets ${_target})
    list(APPEND _runtime_dlls "$<TARGET_RUNTIME_DLLS:${_target}>")
    set(_runtime_directory "$<TARGET_FILE_DIR:${_target}>")
  endif()
endforeach()
foreach(_qt_test Qt6::Test Qt6::QuickTest)
  if(BUILD_TESTING AND TARGET ${_qt_test})
    # Imported Qt targets have no linker language for TARGET_RUNTIME_DLLS.
    # The application graphs above supply their shared Qt dependencies.
    list(APPEND _runtime_dlls "$<TARGET_FILE:${_qt_test}>")
  endif()
endforeach()

if(MINGW)
  get_filename_component(_compiler_directory "${CMAKE_CXX_COMPILER}" DIRECTORY)
  foreach(_name libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll)
    set(_dll "${_compiler_directory}/${_name}")
    if(NOT EXISTS "${_dll}")
      execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-print-file-name=${_name}"
        OUTPUT_VARIABLE _dll OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
    endif()
    if(NOT IS_ABSOLUTE "${_dll}" OR NOT EXISTS "${_dll}")
      message(FATAL_ERROR "The selected MinGW compiler cannot locate ${_name}: ${_dll}")
    endif()
    list(APPEND _runtime_dlls "${_dll}")
  endforeach()
endif()

if(_runtime_targets)
  add_custom_target(yh_windows_runtime ALL
    COMMAND "${CMAKE_COMMAND}" "-DDLLS=${_runtime_dlls}"
      "-DDESTINATION=${_runtime_directory}"
      -P "${CMAKE_CURRENT_LIST_DIR}/CopyWindowsRuntime.cmake"
    DEPENDS ${_runtime_targets}
    COMMENT "Deploying matching Qt and compiler DLLs for build-tree executables"
    VERBATIM)
endif()
