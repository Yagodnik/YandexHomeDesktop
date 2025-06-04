cmake_minimum_required(VERSION 3.16)

include(cmake/qt6-config.cmake)

project(YandexHomeDesktop VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

add_subdirectory(src/api)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_resources(RESOURCES resources/resources.qrc)

qt_add_executable(appYandexHomeDesktop
    src/main.cpp
    ${RESOURCES}
)

qt_add_qml_module(appYandexHomeDesktop
    URI YandexHomeDesktop
    VERSION 1.0
    QML_FILES
        src/qml/ui/HeadingText.qml
        src/qml/ui/DefaultText.qml
        src/qml/ui/SubheadingText.qml
        src/qml/ui/SelectDialog.qml
        src/qml/components/TopBar.qml
        src/qml/pages/DevicesPage.qml
        src/qml/pages/ScenariosPage.qml
        src/qml/pages/SettingsPage.qml
        src/qml/MyTab.qml
        src/qml/MyTabs.qml
        src/qml/Main.qml
)

qt_add_resources(RESOURCES resources.qrc)

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

target_link_libraries(appYandexHomeDesktop
    PRIVATE Qt6::Quick Qt6::Core YandexHomeApi
)

include(GNUInstallDirs)
install(TARGETS appYandexHomeDesktop
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(tests)
