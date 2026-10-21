# Install and deploy each enabled application using its platform runtime.
if(BUILD_DESKTOP_APP)
  install(TARGETS appYandexHomeDesktop
    BUNDLE DESTINATION . COMPONENT DesktopBundle
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  if(APPLE)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
      DESTINATION "YandexHomeDesktop.app/Contents/Resources/licenses" COMPONENT DesktopBundle)
    get_property(_bundle_libraries GLOBAL PROPERTY YH_SDK_TARGETS)
    install(TARGETS ${_bundle_libraries}
      LIBRARY DESTINATION "YandexHomeDesktop.app/Contents/Frameworks" COMPONENT DesktopBundle)
    set_target_properties(appYandexHomeDesktop PROPERTIES
      INSTALL_RPATH "@executable_path/../Frameworks")
    set(_deploy_executables)
    foreach(_console YandexHomeCli YandexHomeRest YandexHomeShortcuts)
      if(TARGET ${_console})
        set_target_properties(${_console} PROPERTIES
          INSTALL_RPATH "@executable_path/../Frameworks;@executable_path/../${CMAKE_INSTALL_LIBDIR}")
        install(TARGETS ${_console} RUNTIME DESTINATION "YandexHomeDesktop.app/Contents/MacOS"
          COMPONENT DesktopBundle)
        # Qt runs macdeployqt from the install prefix, also with --prefix.
        list(APPEND _deploy_executables "-executable=YandexHomeDesktop.app/Contents/MacOS/${_console}")
      endif()
    endforeach()
  endif()
  if(APPLE)
    qt_generate_deploy_qml_app_script(
      TARGET appYandexHomeDesktop
      OUTPUT_SCRIPT deploy_script
      DEPLOY_TOOL_OPTIONS ${_deploy_executables}
    )
  else()
    qt_generate_deploy_qml_app_script(
      TARGET appYandexHomeDesktop
      OUTPUT_SCRIPT deploy_script
    )
  endif()
  if(APPLE)
    install(SCRIPT ${deploy_script} COMPONENT DesktopBundle)
    # Qt 6.9's deploy tool can seal the app before signing the remaining helpers.
    # Sign the complete payload inside out, after all deployment mutations.
    install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SignMacBundle.cmake"
      COMPONENT DesktopBundle)
  else()
    install(SCRIPT ${deploy_script})
  endif()
  # The deploy tool can print errors while returning success. Verify that all
  # packaged entry points can actually load their dependencies.
  set(YH_INSTALLED_CONSOLE_DIR "${CMAKE_INSTALL_BINDIR}")
  if(APPLE)
    set(YH_INSTALLED_CONSOLE_DIR "YandexHomeDesktop.app/Contents/MacOS")
  endif()
  set(YH_INSTALLED_CONSOLES)
  foreach(_console YandexHomeCli YandexHomeRest YandexHomeShortcuts)
    if(TARGET ${_console})
      list(APPEND YH_INSTALLED_CONSOLES "${_console}${CMAKE_EXECUTABLE_SUFFIX}")
    endif()
  endforeach()
  configure_file(cmake/CheckConsoleInstall.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/CheckConsoleInstall.cmake" @ONLY)
  # Windows console deployment runs below, after the GUI deployment.
  if(APPLE)
    install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CheckConsoleInstall.cmake" COMPONENT DesktopBundle)
  endif()
endif()

# Console-only Windows packages need their own Qt runtime deployment. REST's
# HttpServer/WebSockets dependencies are absent from the GUI dependency graph.
if(WIN32)
  foreach(_console YandexHomeCli YandexHomeRest YandexHomeShortcuts)
    if(TARGET ${_console})
      qt_generate_deploy_app_script(TARGET ${_console} OUTPUT_SCRIPT _console_deploy)
      install(SCRIPT ${_console_deploy})
    endif()
  endforeach()
  if(BUILD_DESKTOP_APP)
    install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CheckConsoleInstall.cmake")
  endif()
endif()
