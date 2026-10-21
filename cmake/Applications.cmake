# Shared application bootstraps and the CLI, REST, and shortcut executables.
add_library(AppStartup STATIC
  src/app/common/StartupOptions.cpp src/app/common/StartupOptions.h
  src/app/common/AppTranslations.h)
target_link_libraries(AppStartup PUBLIC Qt6::Core)

add_library(AppShortcuts SHARED
  src/services/ShortcutActionService.h src/services/ShortcutActionService.cpp
  src/services/ShortcutRuntime.h src/services/ShortcutRuntime.cpp
  src/services/ShortcutControlService.h src/services/ShortcutControlService.cpp)
target_link_libraries(AppShortcuts PUBLIC AppServices WorkerControl AppRuntime)
yh_shared_library(AppShortcuts)
add_library(ShortcutControlBootstrap STATIC src/app/shortcuts/ShortcutConfiguration.h src/app/shortcuts/ShortcutConfiguration.cpp)
target_link_libraries(ShortcutControlBootstrap PUBLIC AppStartup AppShortcuts)

qt_add_resources(DEBUG_FIXTURE_RESOURCES resources/debug/debug.qrc)
if(BUILD_SHORTCUTS_APP)
  include(cmake/QHotkey.cmake)
  yh_add_qhotkey()
  qt_add_executable(YandexHomeShortcuts
    src/app/shortcuts/ShortcutMain.cpp
    src/app/shortcuts/QHotkeyRegistrar.h src/app/shortcuts/QHotkeyRegistrar.cpp)
  set_target_properties(YandexHomeShortcuts PROPERTIES WIN32_EXECUTABLE TRUE)
  target_compile_definitions(YandexHomeShortcuts PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
  target_link_libraries(YandexHomeShortcuts PRIVATE ShortcutControlBootstrap QHotkey::QHotkey
    YandexAuth YandexApiAdapter "$<$<CONFIG:Debug>:FixtureApi>")
  target_sources(YandexHomeShortcuts PRIVATE "$<$<CONFIG:Debug>:${DEBUG_FIXTURE_RESOURCES}>")
  if(APPLE)
    target_sources(YandexHomeShortcuts PRIVATE src/app/shortcuts/ShortcutWorkerMac.mm)
    target_link_libraries(YandexHomeShortcuts PRIVATE "-framework AppKit")
  endif()
  install(TARGETS YandexHomeShortcuts RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
add_library(RestControlBootstrap STATIC src/app/rest/RestConfiguration.cpp src/app/rest/RestConfiguration.h
  src/app/rest/RestCommand.cpp src/app/rest/RestCommand.h
  src/app/rest/RestServerConfiguration.cpp src/app/rest/RestServerConfiguration.h)
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
  if(APPLE)
    # Give the helper its own background identity even inside the GUI bundle.
    configure_file(macos/RestInfo.plist.in "${CMAKE_CURRENT_BINARY_DIR}/RestInfo.plist" @ONLY)
    target_link_options(YandexHomeRest PRIVATE
      "LINKER:-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_BINARY_DIR}/RestInfo.plist")
    set_property(TARGET YandexHomeRest APPEND PROPERTY LINK_DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/RestInfo.plist")
  endif()
  install(TARGETS YandexHomeRest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  if(TARGET YandexHomeCli)
    add_dependencies(YandexHomeCli YandexHomeRest)
  endif()
endif()
