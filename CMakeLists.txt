cmake_minimum_required(VERSION 3.18)

project(YandexHomeDesktop VERSION 0.1 LANGUAGES CXX)

if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick Network NetworkAuth Gui QuickControls2 LinguistTools ShaderTools Test)

# Pin dependencies and populate them under the build directory, so a clean
# checkout does not depend on ignored local libraries.
include(FetchContent)
FetchContent_Declare(boost_hana
        GIT_REPOSITORY https://github.com/boostorg/hana.git
        GIT_TAG boost-1.87.0
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR cmake-do-not-add
)
FetchContent_MakeAvailable(boost_hana)

# QtKeychain defaults to Qt 5 and builds optional translations and tests.
set(BUILD_WITH_QT6 ON)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TRANSLATIONS OFF)
set(BUILD_TESTING OFF)
FetchContent_Declare(qtkeychain
        GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
        GIT_TAG aa6da344e1a20b9194e12bace3665caeea6b6304
        GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(qtkeychain)
unset(BUILD_WITH_QT6)
unset(BUILD_SHARED_LIBS)
unset(BUILD_TRANSLATIONS)
unset(BUILD_TESTING)

include(CTest)

qt_standard_project_setup(REQUIRES 6.8)
qt_add_resources(RESOURCES resources/resources.qrc)

include_directories(${boost_hana_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/app)

add_subdirectory(src/api)
add_subdirectory(src/auth)
add_subdirectory(src/models)
add_subdirectory(src/platform)
add_subdirectory(src/utils)
add_subdirectory(src/iot)

qt_add_executable(appYandexHomeDesktop
        src/main.cpp
        src/utils/Router.cpp
        src/utils/Router.h

        ${RESOURCES}
        src/cli/CLI.cpp
        src/cli/CLI.h
        src/cli/OnOffExecutor.cpp
        src/cli/OnOffExecutor.h
        src/cli/IExecutor.cpp
        src/cli/IExecutor.h
        src/cli/ListDevicesCommand.cpp
        src/cli/ListDevicesCommand.h
        src/cli/ResetCommand.cpp
        src/cli/ResetCommand.h
        src/cli/ICommand.h
        src/app/AppContext.h
        src/app/AppContext.cpp
        src/cli/AccountInfoCommand.cpp
        src/cli/AccountInfoCommand.h
        src/app/CliApp.cpp
        src/app/CliApp.h
        src/app/GuiApp.cpp
        src/app/GuiApp.h
)

if (APPLE)
  set_target_properties(appYandexHomeDesktop PROPERTIES
          MACOSX_BUNDLE TRUE
          MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist"
          MACOSX_BUNDLE_ICON_FILE "YandexHomeDesktop.icns"
  )

  set_source_files_properties(resources/YandexHomeDesktop.icns PROPERTIES
          MACOSX_PACKAGE_LOCATION "Resources"
  )

  target_sources(appYandexHomeDesktop PRIVATE resources/YandexHomeDesktop.icns)
endif ()

add_subdirectory(src/qml/)
qt_add_shaders(appYandexHomeDesktop "highlight_shader"
        PREFIX "/shaders"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}/src/qml/shaders"
        GLSL "310 es"
        HLSL 50
        MSL 21
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/qml/shaders/highlight.frag"
)

# Embed a credential-free template by default. Developers can configure an
# ignored local OAuth file without making it a build prerequisite for CI.
set(YH_AUTH_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/auth/example.json"
        CACHE FILEPATH "OAuth configuration JSON to embed in the application")
if (NOT EXISTS "${YH_AUTH_CONFIG_FILE}")
  message(FATAL_ERROR "OAuth configuration not found: ${YH_AUTH_CONFIG_FILE}")
endif ()
set_source_files_properties("${YH_AUTH_CONFIG_FILE}" PROPERTIES
        QT_RESOURCE_ALIAS "auth/secrets.json")
qt_add_resources(appYandexHomeDesktop "auth_config"
        PREFIX "/"
        FILES "${YH_AUTH_CONFIG_FILE}")

qt_add_qml_module(appYandexHomeDesktop
        URI YandexHomeDesktop
        VERSION 1.0
        QML_FILES
        src/qml/Main.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appYandexHomeDesktop PROPERTIES
        #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appYandexHomeDesktop
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(appYandexHomeDesktop PRIVATE
        Qt6::Quick
        Qt6::Core
        Qt6::NetworkAuth
        Qt6::Gui
        Qt6::QuickControls2

        YandexHomeDesktopUi
        YandexHomeDesktopPages
        YandexHomeDesktopComponents

        YandexApiAdapter
        YandexAuth
        Models
        PlatformService
        Utils
        Iot
)

file(GLOB_RECURSE QML_TRANSLATION_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/qml/*.qml")

qt_add_translations(appYandexHomeDesktop
        TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/YandexHomeDesktop_en.ts"
        SOURCE_TARGETS appYandexHomeDesktop
        SOURCES
        ${QML_TRANSLATION_SOURCES}
        translations/DataStrings.cpp
        src/models/DeviceModel/CapabilitiesModel.cpp
        src/models/DeviceModel/PropertiesModel.cpp
        RESOURCE_PREFIX "/i18n"
)
include(GNUInstallDirs)
install(TARGETS appYandexHomeDesktop
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(WIN32)
  set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
  target_sources(appYandexHomeDesktop PRIVATE ${APP_ICON_RESOURCE})
endif()

if(UNIX AND NOT APPLE)
else()
  set_target_properties(appYandexHomeDesktop PROPERTIES
          OUTPUT_NAME "YandexHomeDesktop"
          MACOSX_BUNDLE TRUE
  )
endif()

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()
