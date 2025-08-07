cmake_minimum_required(VERSION 3.16)

include(cmake/qt6-config.cmake OPTIONAL)

project(YandexHomeDesktop VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network NetworkAuth Gui QuickControls2)

qt_standard_project_setup(REQUIRES 6.8)
qt_add_resources(RESOURCES resources/resources.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
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
add_subdirectory(src/qml/shaders)

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
        YandexHomeDesktopIotControls

        YandexHomeApi
        YandexAuth
        Models
        PlatformService
        Utils
        Iot
)

add_dependencies(appYandexHomeDesktop compile_shaders)

include(GNUInstallDirs)
install(TARGETS appYandexHomeDesktop
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(UNIX AND NOT APPLE)
else()
  set_target_properties(appYandexHomeDesktop PROPERTIES
          OUTPUT_NAME "YandexHomeDesktop"
          MACOSX_BUNDLE TRUE
  )
endif()

add_subdirectory(tests)
