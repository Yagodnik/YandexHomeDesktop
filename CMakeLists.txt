cmake_minimum_required(VERSION 3.28)

project(YandexHomeDesktop VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

option(YH_ENABLE_PCH "Use precompiled headers for C++ targets" ON)

include(CTest)
include(FetchContent)

set(_desktop_default OFF)
if(APPLE OR WIN32)
  set(_desktop_default ON)
endif()
option(BUILD_DESKTOP_APP "Build the Windows or macOS desktop application" ${_desktop_default})

if(BUILD_DESKTOP_APP AND NOT (APPLE OR WIN32))
  message(FATAL_ERROR "The desktop application is supported on Windows and macOS only")
endif()

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Gui Network)
if(BUILD_TESTING)
  find_package(Qt6 6.9 REQUIRED COMPONENTS Test)
endif()
if(BUILD_DESKTOP_APP)
  find_package(Qt6 6.9 REQUIRED COMPONENTS Quick NetworkAuth QuickControls2 Widgets Svg LinguistTools)
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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/app)

add_library(AppStartup STATIC src/app/StartupOptions.cpp src/app/StartupOptions.h)
target_link_libraries(AppStartup PUBLIC Qt6::Core)

add_subdirectory(src/api)
add_subdirectory(src/iot)
add_subdirectory(src/services)
add_subdirectory(src/cli)
add_subdirectory(src/models)
add_subdirectory(src/utils)

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

  # Keep QtKeychain's install rules and tests out of this application's package.
  FetchContent_Declare(qtkeychain
    GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
    GIT_TAG 0.15.0
    GIT_SHALLOW TRUE
    EXCLUDE_FROM_ALL
  )
  set(BUILD_WITH_QT6 ON)
  set(BUILD_SHARED_LIBS OFF)
  set(BUILD_TRANSLATIONS OFF)
  set(BUILD_TEST_APPLICATION OFF)
  set(_project_build_testing ${BUILD_TESTING})
  set(BUILD_TESTING OFF)
  FetchContent_MakeAvailable(qtkeychain)
  set(BUILD_TESTING ${_project_build_testing})
  unset(_project_build_testing)
  unset(BUILD_WITH_QT6)
  unset(BUILD_SHARED_LIBS)
  unset(BUILD_TRANSLATIONS)
  unset(BUILD_TEST_APPLICATION)

  add_subdirectory(src/auth)
  target_compile_definitions(YandexAuth PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
  add_subdirectory(src/platform)
  add_subdirectory(src/qml)

  if(BUILD_TESTING)
    find_package(Qt6 6.9 REQUIRED COMPONENTS QuickTest)
    target_compile_definitions(ApiFixtureTests PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
    target_link_libraries(ApiFixtureTests PRIVATE "$<$<CONFIG:Debug>:YandexAuth>")
    target_sources(HomeTests PRIVATE tests/QmlTestEnvironment.cpp tests/QmlTestEnvironment.h)
    target_compile_definitions(HomeTests PRIVATE
      HOME_QML_TESTS HOME_IMPORT_PATH="${CMAKE_CURRENT_BINARY_DIR}")
    target_link_libraries(HomeTests PRIVATE Qt6::Quick Qt6::QuickControls2 Iot)
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
  qt_add_resources(DEBUG_FIXTURE_RESOURCES resources/debug/debug.qrc)
  if(BUILD_TESTING)
    target_sources(QmlPageLoadTest PRIVATE ${RESOURCES})
    target_sources(QmlLayoutTests PRIVATE ${RESOURCES})
    target_sources(ScenariosTests PRIVATE ${RESOURCES})
    target_sources(HomeTests PRIVATE ${RESOURCES})
  endif()
  qt_add_executable(appYandexHomeDesktop
    src/main.cpp
    src/utils/Router.cpp
    src/utils/Router.h
    ${RESOURCES}
    src/app/AppContext.cpp
    src/app/AppContext.h
    src/app/CliApp.h
    src/app/GuiApp.cpp
    src/app/GuiApp.h
  )
  add_library(CliBootstrap STATIC src/app/CliApp.cpp src/app/CliApp.h)
  target_compile_definitions(CliBootstrap PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
  target_link_libraries(CliBootstrap PUBLIC AppCli AppStartup YandexAuth YandexApiAdapter Utils
    "$<$<CONFIG:Debug>:FixtureApi>")
  add_executable(YandexHomeCli src/app/CliMain.cpp)
  target_compile_definitions(YandexHomeCli PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
  target_link_libraries(YandexHomeCli PRIVATE CliBootstrap)
  target_sources(YandexHomeCli PRIVATE "$<$<CONFIG:Debug>:${DEBUG_FIXTURE_RESOURCES}>")

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

  qt_add_qml_module(appYandexHomeDesktop
    URI YandexHomeDesktop
    VERSION 1.0
    QML_FILES src/qml/Main.qml
  )

  file(GLOB_RECURSE QML_TRANSLATION_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/qml/*.qml")
  qt_add_translations(appYandexHomeDesktop TARGETS YandexHomeCli
    TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/YandexHomeDesktop_en.ts"
    SOURCE_TARGETS appYandexHomeDesktop AppCli CliBootstrap IotCore
    SOURCES
      ${QML_TRANSLATION_SOURCES}
      translations/DataStrings.cpp
      src/models/DeviceModel/CapabilitiesModel.cpp
      src/models/DeviceModel/PropertiesModel.cpp
      src/models/ScenariosModel/ScenariosViewModel.cpp
    RESOURCE_PREFIX "/i18n"
  )

  set_target_properties(appYandexHomeDesktop PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "YandexHomeDesktop"
  )

  target_link_libraries(appYandexHomeDesktop PRIVATE
    Qt6::Quick Qt6::Core Qt6::NetworkAuth Qt6::Gui Qt6::QuickControls2 Qt6::Widgets Qt6::Svg
    YandexHomeDesktopUi YandexHomeDesktopPages
    YandexHomeDesktopComponents
    YandexApiAdapter YandexAuth Models PlatformService Utils Iot
    AppStartup CliBootstrap "$<$<CONFIG:Debug>:FixtureApi>"
  )

  if(BUILD_TESTING)
    foreach(_cli_target YandexHomeCli appYandexHomeDesktop)
      add_test(NAME CliProcess_${_cli_target} COMMAND "${CMAKE_COMMAND}"
        "-DAPP_PATH=$<TARGET_FILE:${_cli_target}>"
        "-DCONFIGURATION=$<CONFIG>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/CliSmoke.cmake")
      set_tests_properties(CliProcess_${_cli_target} PROPERTIES TIMEOUT 30)
    endforeach()
    set(_fixture_debug_configs)
    set(_fixture_release_configs)
    if(CMAKE_CONFIGURATION_TYPES)
      set(_fixture_debug_configs CONFIGURATIONS Debug)
      set(_fixture_release_configs CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
    endif()
    if(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE STREQUAL "Debug")
      add_test(NAME DebugFixtureCli ${_fixture_debug_configs}
        COMMAND appYandexHomeDesktop --fake-api --list-devices)
      set_tests_properties(DebugFixtureCli PROPERTIES
        TIMEOUT 15 PASS_REGULAR_EXPRESSION "Desk lamp"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    endif()
    if(CMAKE_CONFIGURATION_TYPES OR NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
      add_test(NAME ReleaseRejectsFixture ${_fixture_release_configs} COMMAND "${CMAKE_COMMAND}"
        "-DAPP_PATH=$<TARGET_FILE:appYandexHomeDesktop>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/RejectFakeApi.cmake")
      set_tests_properties(ReleaseRejectsFixture PROPERTIES
        TIMEOUT 15 ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    endif()
  endif()

  include(GNUInstallDirs)
  install(TARGETS appYandexHomeDesktop
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  if(APPLE)
    set_target_properties(YandexHomeCli PROPERTIES INSTALL_RPATH "@executable_path/../Frameworks")
    install(TARGETS YandexHomeCli RUNTIME DESTINATION "YandexHomeDesktop.app/Contents/MacOS")
  else()
    install(TARGETS YandexHomeCli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
  if(APPLE)
    qt_generate_deploy_qml_app_script(
      TARGET appYandexHomeDesktop
      OUTPUT_SCRIPT deploy_script
      DEPLOY_TOOL_OPTIONS "-codesign=-"
    )
  else()
    qt_generate_deploy_qml_app_script(
      TARGET appYandexHomeDesktop
      OUTPUT_SCRIPT deploy_script
    )
  endif()
  install(SCRIPT ${deploy_script})

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

if(YH_ENABLE_PCH)
  foreach(_pch_target YandexApiAdapter Models Utils Iot AppTests appYandexHomeDesktop)
    if(TARGET ${_pch_target})
      target_precompile_headers(${_pch_target} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/serialization/PrecompiledHeaders.h")
    endif()
  endforeach()
endif()
