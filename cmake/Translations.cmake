# Translate each enabled entry point without linking the GUI.
set(_translation_targets)
foreach(_app appYandexHomeDesktop YandexHomeCli YandexHomeRest YandexHomeShortcuts)
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
  set(_translation_sources ${_gui_translation_sources} AppStartup AppCli AppServices AppRestControl AppShortcuts IotCore YandexApiAdapter RestControlBootstrap)
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
      src/models/KeybindSettingsModel.cpp src/models/ShortcutViewModel.cpp
      src/models/SettingsTransferModel.cpp
      src/models/DeviceModel/CameraSnapshotModel.cpp
      src/models/McpViewModel.cpp
    SOURCE_TARGETS ${_translation_sources}
    RESOURCE_PREFIX "/i18n")
endif()
