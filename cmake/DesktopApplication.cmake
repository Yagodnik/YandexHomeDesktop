# Desktop executable, QML modules, and bundled application resources.
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

qt_add_resources(DESKTOP_RESOURCES resources/resources.qrc)
qt_add_executable(appYandexHomeDesktop
  src/app/gui/GuiMain.cpp
  src/utils/Router.cpp
  src/utils/Router.h
  ${DESKTOP_RESOURCES}
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
if(TARGET YandexHomeMcp)
  add_dependencies(appYandexHomeDesktop YandexHomeMcp)
  target_link_libraries(appYandexHomeDesktop PRIVATE McpControl)
  target_compile_definitions(appYandexHomeDesktop PRIVATE YH_MCP_ENABLED)
endif()
if(TARGET YandexHomeShortcuts)
  add_dependencies(appYandexHomeDesktop YandexHomeShortcuts)
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
  AppStartup RestControlBootstrap ShortcutControlBootstrap "$<$<CONFIG:Debug>:FixtureApi>"
)
