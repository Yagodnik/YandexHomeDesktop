cmake_minimum_required(VERSION 3.16)

include(cmake/qt6-config.cmake)

project(YandexHomeDesktop VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network NetworkAuth)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src/api)
add_subdirectory(src/auth)
add_subdirectory(src/models)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_resources(RESOURCES resources/resources.qrc)

qt_add_executable(appYandexHomeDesktop
    src/main.cpp
    src/Router.cpp

    ${RESOURCES}
)

qt_add_qml_module(YandexHomeDesktopUi
        URI YandexHomeDesktop.Ui
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/YandexHomeDesktop/Ui"
        VERSION 1.0
        QML_FILES
        src/qml/ui/HeadingText.qml
        src/qml/ui/DefaultText.qml
        src/qml/ui/SubheadingText.qml
        src/qml/ui/SelectDialog.qml
        src/qml/ui/MyButton.qml
        src/qml/ui/MyProgressIndicator.qml
        src/qml/ui/MySwitch.qml
        src/qml/ui/AnimatedText.qml
        src/qml/ui/MyTab.qml
        src/qml/ui/MyTabs.qml
)

qt_add_qml_module(YandexHomeDesktopPages
        URI YandexHomeDesktop.Pages
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/YandexHomeDesktop/Pages"
        VERSION 1.0
        QML_FILES
        src/qml/pages/DevicesPage.qml
        src/qml/pages/ScenariosPage.qml
        src/qml/pages/SettingsPage.qml
        src/qml/pages/AuthPage.qml
        src/qml/pages/ErrorPage.qml
        src/qml/pages/AuthCanceledPage.qml
        src/qml/pages/LoadingPage.qml
        src/qml/pages/MainPage.qml
)

qt_add_qml_module(YandexHomeDesktopComponents
        URI YandexHomeDesktop.Components
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/YandexHomeDesktop/Components"
        VERSION 1.0
        QML_FILES
        src/qml/components/TopBar.qml
        src/qml/components/ScenarioDelegate.qml
)

qt_add_qml_module(appYandexHomeDesktop
        URI YandexHomeDesktop
        VERSION 1.0
        QML_FILES
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

target_link_libraries(appYandexHomeDesktop PRIVATE
        Qt6::Quick
        Qt6::Core
        Qt6::NetworkAuth

        YandexHomeDesktopUi
        YandexHomeDesktopPages
        YandexHomeDesktopComponents

        YandexHomeApi
        YandexAuth
        Models
)

set(SHADER_SRC
  "src/qml/shaders/highlight.frag"
)
set(SHADER_OUT
  "resources/shaders/highlight.frag.qsb"
)

add_custom_command(
        OUTPUT ${SHADER_OUT}
        COMMAND qsb
        "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SRC}"
        -o "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_OUT}"
        --glsl 310es
        --hlsl 50
        --msl 21
        DEPENDS ${SHADER_SRC}
        COMMENT "Compiling shader ${SHADER_SRC} to ${SHADER_OUT}"
        VERBATIM
)

add_custom_target(compile_shaders ALL DEPENDS ${SHADER_OUT})

add_dependencies(appYandexHomeDesktop compile_shaders)

include(GNUInstallDirs)
install(TARGETS appYandexHomeDesktop
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_subdirectory(tests)
