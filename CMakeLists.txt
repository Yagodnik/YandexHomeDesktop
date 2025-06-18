cmake_minimum_required(VERSION 3.16)

include(cmake/qt6-config.cmake)

project(YandexHomeDesktop VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network NetworkAuth QuickControls2)

qt_standard_project_setup(REQUIRES 6.8)
qt_add_resources(RESOURCES resources/resources.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src/api)
add_subdirectory(src/auth)
add_subdirectory(src/models)
add_subdirectory(src/platform)

qt_add_executable(appYandexHomeDesktop
        src/main.cpp
        src/Router.cpp

        ${RESOURCES}
        src/Themes.cpp
        src/Themes.h
)

if(APPLE)
  set_target_properties(appYandexHomeDesktop PROPERTIES
          MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/macos/Info.plist"
  )
endif()

add_subdirectory(src/qml)
add_subdirectory(src/qml/shaders)

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
        Qt6::QuickControls2

        YandexHomeDesktopUi
        YandexHomeDesktopPages
        YandexHomeDesktopComponents

        YandexHomeApi
        YandexAuth
        Models
        PlatformService
)

add_dependencies(appYandexHomeDesktop compile_shaders)

include(GNUInstallDirs)
install(TARGETS appYandexHomeDesktop
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_subdirectory(tests)