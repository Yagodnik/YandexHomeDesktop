cmake_minimum_required(VERSION 3.28)

project(YandexHomeDesktop VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(YH_ENABLE_PCH "Use precompiled headers for C++ targets" ON)

include(CTest)
include(FetchContent)
include(cmake/YandexHomeLibraries.cmake)

# DLLs and executables share a directory on Windows, including test programs.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(APPLE)
  set(CMAKE_INSTALL_RPATH "@loader_path;@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(UNIX)
  set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

set(_desktop_default OFF)
if(APPLE OR WIN32)
  set(_desktop_default ON)
endif()
option(BUILD_DESKTOP_APP "Build the Windows or macOS desktop application" ${_desktop_default})

option(BUILD_CLI_APP "Build the console CLI" ON)
option(BUILD_REST_APP "Build the headless REST worker" ON)
option(YH_BUILD_REST_ADAPTER "Build the reusable REST adapter" ${BUILD_REST_APP})
if(BUILD_REST_APP AND NOT YH_BUILD_REST_ADAPTER)
  message(FATAL_ERROR "BUILD_REST_APP requires YH_BUILD_REST_ADAPTER")
endif()

if(BUILD_DESKTOP_APP AND NOT (APPLE OR WIN32))
  message(FATAL_ERROR "The desktop application is supported on Windows and macOS only")
endif()

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Network)
if(YH_BUILD_REST_ADAPTER)
  find_package(Qt6 6.9 REQUIRED COMPONENTS HttpServer)
endif()
if(BUILD_DESKTOP_APP OR BUILD_TESTING)
  find_package(Qt6 6.9 REQUIRED COMPONENTS Gui)
endif()
if(BUILD_TESTING)
  find_package(Qt6 6.9 REQUIRED COMPONENTS Test)
endif()
if(BUILD_DESKTOP_APP)
  find_package(Qt6 6.9 REQUIRED COMPONENTS Quick NetworkAuth QuickControls2 Widgets Svg LinguistTools)
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

add_library(AppStartup STATIC
  src/app/common/StartupOptions.cpp src/app/common/StartupOptions.h
  src/app/common/AppTranslations.h)
target_link_libraries(AppStartup PUBLIC Qt6::Core)

# Keep QtKeychain's install rules and tests out of this application's package.
FetchContent_Declare(qtkeychain
  GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
  GIT_TAG 0.15.0
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

add_subdirectory(src/api)
add_subdirectory(src/iot)
add_subdirectory(src/services)
if(YH_BUILD_REST_ADAPTER)
  add_subdirectory(src/rest)
endif()
add_subdirectory(src/cli)
add_subdirectory(src/auth)
add_subdirectory(src/utils)
if(BUILD_DESKTOP_APP OR BUILD_TESTING)
  add_subdirectory(src/models)
endif()

qt_add_resources(DEBUG_FIXTURE_RESOURCES resources/debug/debug.qrc)
add_library(RestControlBootstrap STATIC src/app/rest/RestConfiguration.cpp src/app/rest/RestConfiguration.h
  src/app/rest/RestCommand.cpp src/app/rest/RestCommand.h)
target_link_libraries(RestControlBootstrap PUBLIC AppStartup AppRestControl)

if(BUILD_CLI_APP)
  add_library(CliBootstrap STATIC src/app/cli/CliApp.cpp src/app/cli/CliApp.h
    src/app/cli/CliApplication.h)
  target_compile_definitions(CliBootstrap PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
  target_link_libraries(CliBootstrap PUBLIC AppCli AppStartup RestControlBootstrap YandexAuth YandexApiAdapter AppRuntime
    "$<$<CONFIG:Debug>:FixtureApi>")
  add_executable(YandexHomeCli src/app/cli/CliMain.cpp)
  target_compile_definitions(YandexHomeCli PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
  target_link_libraries(YandexHomeCli PRIVATE CliBootstrap)
  target_sources(YandexHomeCli PRIVATE "$<$<CONFIG:Debug>:${DEBUG_FIXTURE_RESOURCES}>")
  install(TARGETS YandexHomeCli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
if(BUILD_REST_APP)
  add_library(RestBootstrap STATIC src/app/rest/RestApp.cpp src/app/rest/RestApp.h)
  target_compile_definitions(RestBootstrap PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
  target_link_libraries(RestBootstrap PUBLIC AppRest AppStartup RestControlBootstrap YandexAuth YandexApiAdapter AppRuntime
    "$<$<CONFIG:Debug>:FixtureApi>")
  add_executable(YandexHomeRest src/app/rest/RestMain.cpp)
  target_compile_definitions(YandexHomeRest PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
  target_link_libraries(YandexHomeRest PRIVATE RestBootstrap)
  target_sources(YandexHomeRest PRIVATE "$<$<CONFIG:Debug>:${DEBUG_FIXTURE_RESOURCES}>")
  install(TARGETS YandexHomeRest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  if(TARGET YandexHomeCli)
    add_dependencies(YandexHomeCli YandexHomeRest)
  endif()
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(BUILD_DESKTOP_APP)
  set(YH_AUTH_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/auth/example.json"
    CACHE FILEPATH "OAuth configuration JSON to embed in the application")
  if(AUTH_CONFIG_FILE)
    set(YH_AUTH_CONFIG_FILE "${AUTH_CONFIG_FILE}")
  endif()
  if(NOT EXISTS "${YH_AUTH_CONFIG_FILE}")
    message(FATAL_ERROR "OAuth configuration file does not exist: ${YH_AUTH_CONFIG_FILE}")
  endif()

  add_subdirectory(src/platform)
  add_subdirectory(src/qml)

  if(BUILD_TESTING)
    target_compile_definitions(ApiFixtureTests PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
    target_link_libraries(ApiFixtureTests PRIVATE "$<$<CONFIG:Debug>:YandexAuth>")
    target_sources(HomeTests PRIVATE tests/QmlTestEnvironment.cpp tests/QmlTestEnvironment.h)
    target_compile_definitions(HomeTests PRIVATE
      HOME_QML_TESTS HOME_IMPORT_PATH="${CMAKE_CURRENT_BINARY_DIR}")
    target_link_libraries(HomeTests PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::QuickTest Iot)
    add_dependencies(HomeTests YandexHomeDesktopComponentsplugin
      YandexHomeDesktopUiplugin YandexHomeDesktopPagesplugin)
    target_compile_definitions(ScenariosTests PRIVATE
      SCENARIOS_QML_TESTS SCENARIOS_IMPORT_PATH="${CMAKE_CURRENT_BINARY_DIR}")
    target_link_libraries(ScenariosTests PRIVATE Qt6::Quick Qt6::QuickControls2)
    add_dependencies(ScenariosTests YandexHomeDesktopComponentsplugin
      YandexHomeDesktopUiplugin YandexHomeDesktopPagesplugin)
    add_executable(QmlPageLoadTest tests/QmlPageLoadTest.cpp
      tests/QmlTestEnvironment.cpp tests/QmlTestEnvironment.h)
    target_link_libraries(QmlPageLoadTest PRIVATE Qt6::Gui Qt6::Qml Qt6::Test Models Utils Iot)
    add_dependencies(QmlPageLoadTest YandexHomeDesktopComponentsplugin
      YandexHomeDesktopUiplugin YandexHomeDesktopPagesplugin)
    foreach(_qml_page AuthPage AuthCanceledPage ErrorPage LoadingPage MainPage
        DevicePage DevicesPage ScenariosPage SettingsPage)
      add_test(NAME ${_qml_page}Loads COMMAND QmlPageLoadTest
        "${CMAKE_CURRENT_SOURCE_DIR}/src/qml/pages/${_qml_page}.qml"
        "${CMAKE_CURRENT_BINARY_DIR}")
      set_tests_properties(${_qml_page}Loads PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_QUICK_CONTROLS_STYLE=Basic")
    endforeach()

    add_executable(QmlLayoutTests tests/QmlLayoutTests.cpp
      tests/QmlTestEnvironment.cpp tests/QmlTestEnvironment.h)
    target_link_libraries(QmlLayoutTests PRIVATE Qt6::QuickTest Qt6::Qml Qt6::Gui Models Utils Iot)
    add_dependencies(QmlLayoutTests YandexHomeDesktopComponentsplugin
      YandexHomeDesktopUiplugin YandexHomeDesktopPagesplugin)
    add_test(NAME QmlLayouts COMMAND QmlLayoutTests
      -input "${CMAKE_CURRENT_SOURCE_DIR}/tests/qml"
      -import "${CMAKE_CURRENT_BINARY_DIR}")
    set_tests_properties(QmlLayouts PROPERTIES
      ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_QUICK_CONTROLS_STYLE=Basic")
  endif()

  qt_add_resources(RESOURCES resources/resources.qrc)
  if(BUILD_TESTING)
    target_sources(QmlPageLoadTest PRIVATE ${RESOURCES})
    target_sources(QmlLayoutTests PRIVATE ${RESOURCES})
    target_sources(ScenariosTests PRIVATE ${RESOURCES})
    target_sources(HomeTests PRIVATE ${RESOURCES})
  endif()
  qt_add_executable(appYandexHomeDesktop
    src/app/gui/GuiMain.cpp
    src/utils/Router.cpp
    src/utils/Router.h
    ${RESOURCES}
    src/app/gui/AppContext.cpp
    src/app/gui/AppContext.h
    src/app/gui/GuiApp.cpp
    src/app/gui/GuiApp.h
  )
  target_compile_definitions(appYandexHomeDesktop PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
  target_sources(appYandexHomeDesktop PRIVATE "$<$<CONFIG:Debug>:${DEBUG_FIXTURE_RESOURCES}>")

  set_source_files_properties("${YH_AUTH_CONFIG_FILE}" PROPERTIES QT_RESOURCE_ALIAS "auth/secrets.json")
  qt_add_resources(appYandexHomeDesktop "oauth_config"
    PREFIX "/"
    FILES "${YH_AUTH_CONFIG_FILE}"
  )
  if(APPLE)
    configure_file(macos/Info.plist "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)
    set_target_properties(appYandexHomeDesktop PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
      MACOSX_BUNDLE_ICON_FILE "YandexHomeDesktop.icns"
    )
    set_source_files_properties(resources/YandexHomeDesktop.icns PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(appYandexHomeDesktop PRIVATE resources/YandexHomeDesktop.icns)
  endif()

  if(TARGET YandexHomeRest)
    add_dependencies(appYandexHomeDesktop YandexHomeRest)
  endif()

  qt_add_qml_module(appYandexHomeDesktop
    URI YandexHomeDesktop
    VERSION 1.0
    QML_FILES src/qml/Main.qml
  )

  set_target_properties(appYandexHomeDesktop PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "YandexHomeDesktop"
  )

  target_link_libraries(appYandexHomeDesktop PRIVATE
    Qt6::Quick Qt6::Core Qt6::Gui Qt6::QuickControls2 Qt6::Widgets Qt6::Svg
    YandexHomeDesktopUi YandexHomeDesktopPages
    YandexHomeDesktopComponents
    YandexApiAdapter YandexAuth YandexOAuth Models PlatformService Utils Iot
    AppStartup RestControlBootstrap "$<$<CONFIG:Debug>:FixtureApi>"
  )

  include(GNUInstallDirs)
  install(TARGETS appYandexHomeDesktop
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  if(APPLE)
    get_property(_bundle_libraries GLOBAL PROPERTY YH_SDK_TARGETS)
    install(TARGETS ${_bundle_libraries}
      LIBRARY DESTINATION "YandexHomeDesktop.app/Contents/Frameworks")
    set_target_properties(appYandexHomeDesktop PROPERTIES
      INSTALL_RPATH "@executable_path/../Frameworks")
    set(_deploy_executables)
    foreach(_console YandexHomeCli YandexHomeRest)
      if(TARGET ${_console})
        set_target_properties(${_console} PROPERTIES
          INSTALL_RPATH "@executable_path/../Frameworks;@executable_path/../${CMAKE_INSTALL_LIBDIR}")
        install(TARGETS ${_console} RUNTIME DESTINATION "YandexHomeDesktop.app/Contents/MacOS")
        # Qt runs macdeployqt from the install prefix, also with --prefix.
        list(APPEND _deploy_executables "-executable=YandexHomeDesktop.app/Contents/MacOS/${_console}")
      endif()
    endforeach()
  endif()
  if(APPLE)
    qt_generate_deploy_qml_app_script(
      TARGET appYandexHomeDesktop
      OUTPUT_SCRIPT deploy_script
      DEPLOY_TOOL_OPTIONS "-codesign=-" ${_deploy_executables}
    )
  else()
    qt_generate_deploy_qml_app_script(
      TARGET appYandexHomeDesktop
      OUTPUT_SCRIPT deploy_script
    )
  endif()
  install(SCRIPT ${deploy_script})
  # The deploy tool can print errors while returning success. Verify that all
  # packaged entry points can actually load their dependencies.
  set(YH_INSTALLED_CONSOLE_DIR "${CMAKE_INSTALL_BINDIR}")
  if(APPLE)
    set(YH_INSTALLED_CONSOLE_DIR "YandexHomeDesktop.app/Contents/MacOS")
  endif()
  set(YH_INSTALLED_CONSOLES)
  foreach(_console YandexHomeCli YandexHomeRest)
    if(TARGET ${_console})
      list(APPEND YH_INSTALLED_CONSOLES "${_console}${CMAKE_EXECUTABLE_SUFFIX}")
    endif()
  endforeach()
  configure_file(cmake/CheckConsoleInstall.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/CheckConsoleInstall.cmake" @ONLY)
  # Windows console deployment runs below, after the GUI deployment.
  if(APPLE)
    install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CheckConsoleInstall.cmake")
  endif()

  if(WIN32)
    set(CPACK_GENERATOR NSIS)
    set(CPACK_PACKAGE_NAME YandexHomeDesktop)
    set(CPACK_PACKAGE_VENDOR Yagodnik)
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Yandex smart home desktop application")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_PACKAGE_EXECUTABLES "YandexHomeDesktop" "Yandex Home Desktop")
    set(CPACK_PACKAGE_FILE_NAME "YandexHomeDesktop-${PROJECT_VERSION}-windows-x64")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY YandexHomeDesktop)
    set(CPACK_NSIS_DISPLAY_NAME "Yandex Home Desktop")
    include(CPack)
  endif()
endif()

# Console-only Windows packages need their own Qt runtime deployment. REST's
# HttpServer/WebSockets dependencies are absent from the GUI dependency graph.
if(WIN32)
  foreach(_console YandexHomeCli YandexHomeRest)
    if(TARGET ${_console})
      qt_generate_deploy_app_script(TARGET ${_console} OUTPUT_SCRIPT _console_deploy)
      install(SCRIPT ${_console_deploy})
    endif()
  endforeach()
  if(BUILD_DESKTOP_APP)
    install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CheckConsoleInstall.cmake")
  endif()
endif()

# Translate each enabled entry point without linking the GUI.
set(_translation_targets)
foreach(_app appYandexHomeDesktop YandexHomeCli YandexHomeRest)
  if(TARGET ${_app})
    list(APPEND _translation_targets ${_app})
  endif()
endforeach()
if(_translation_targets)
  find_package(Qt6 6.9 REQUIRED COMPONENTS LinguistTools)
  file(GLOB_RECURSE QML_TRANSLATION_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/qml/*.qml")
  if(BUILD_DESKTOP_APP)
    set(_gui_translation_sources appYandexHomeDesktop)
  endif()
  set(_translation_sources ${_gui_translation_sources} AppStartup AppCli AppServices AppRestControl IotCore YandexApiAdapter RestControlBootstrap)
  foreach(_target CliBootstrap RestBootstrap AppRest)
    if(TARGET ${_target})
      list(APPEND _translation_sources ${_target})
    endif()
  endforeach()
  qt_add_translations(TARGETS ${_translation_targets}
    TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/YandexHomeDesktop_en.ts"
    SOURCES ${QML_TRANSLATION_SOURCES} translations/DataStrings.cpp
      src/models/DeviceModel/CapabilitiesModel.cpp src/models/DeviceModel/PropertiesModel.cpp
      src/models/ScenariosModel/ScenariosViewModel.cpp src/models/RestViewModel.cpp
    SOURCE_TARGETS ${_translation_sources}
    RESOURCE_PREFIX "/i18n")
endif()
yh_install_sdk()

if(YH_ENABLE_PCH)
  foreach(_pch_target YandexApiAdapter Models Utils Iot AppTests appYandexHomeDesktop)
    if(TARGET ${_pch_target})
      target_precompile_headers(${_pch_target} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/serialization/PrecompiledHeaders.h")
    endif()
  endforeach()
endif()
