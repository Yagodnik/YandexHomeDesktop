# Required Qt modules and pinned third-party sources.
include(FetchContent)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Network)
if(YH_BUILD_REST_ADAPTER)
  find_package(Qt6 6.9 REQUIRED COMPONENTS HttpServer)
endif()
if(BUILD_DESKTOP_APP OR BUILD_SHORTCUTS_APP OR BUILD_TESTING)
  find_package(Qt6 6.9 REQUIRED COMPONENTS Gui)
endif()
if(BUILD_TESTING)
  find_package(Qt6 6.9 REQUIRED COMPONENTS Test)
endif()
if(BUILD_DESKTOP_APP)
  # GraphicalEffects imports need ShaderTools at runtime, even without a C++ link.
  find_package(Qt6 6.9 REQUIRED COMPONENTS
    Quick NetworkAuth QuickControls2 Widgets Svg LinguistTools Multimedia
    Core5Compat ShaderTools)
  if(BUILD_TESTING)
    find_package(Qt6 6.9 REQUIRED COMPONENTS QuickTest)
  endif()
endif()
qt_standard_project_setup(REQUIRES 6.8)

# Hana is header-only. SOURCE_SUBDIR prevents building its tests and examples.
FetchContent_Declare(hana
  GIT_REPOSITORY https://github.com/boostorg/hana.git
  GIT_TAG boost-1.90.0
  GIT_SHALLOW TRUE
  SOURCE_SUBDIR no-cmake-subdirectory
)
FetchContent_MakeAvailable(hana)

include_directories(${hana_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Keep QtKeychain's install rules and tests out of this application's package.
FetchContent_Declare(qtkeychain
  GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
  # 0.16.0 guards MSVC's /utf-8 and uses GCC-compatible flags for MinGW.
  GIT_TAG 0.16.0
  GIT_SHALLOW TRUE
  EXCLUDE_FROM_ALL
)
set(BUILD_WITH_QT6 ON)
set(_project_shared_libs ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TRANSLATIONS OFF)
set(BUILD_TEST_APPLICATION OFF)
set(_project_build_testing ${BUILD_TESTING})
set(BUILD_TESTING OFF)
FetchContent_MakeAvailable(qtkeychain)
set(BUILD_TESTING ${_project_build_testing})
unset(_project_build_testing)
unset(BUILD_WITH_QT6)
set(BUILD_SHARED_LIBS ${_project_shared_libs})
unset(_project_shared_libs)
unset(BUILD_TRANSLATIONS)
unset(BUILD_TEST_APPLICATION)

if(BUILD_TESTING)
  find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
endif()
