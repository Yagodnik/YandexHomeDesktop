yh_add_test(NAME SpixSmoke COMMAND "${Python3_EXECUTABLE}"
  "${CMAKE_CURRENT_SOURCE_DIR}/spix_smoke.py" "$<TARGET_FILE:appYandexHomeDesktop>")
set_tests_properties(SpixSmoke PROPERTIES RESOURCE_LOCK spix_port_9000 TIMEOUT 90)

function(yh_add_spix_fixture_test name script)
  yh_add_test(NAME "${name}" CONFIGURATIONS Debug COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${script}" "$<TARGET_FILE:appYandexHomeDesktop>" ${ARGN})
  set_tests_properties("${name}" PROPERTIES RESOURCE_LOCK spix_port_9000 TIMEOUT 90)
endfunction()

yh_add_spix_fixture_test(CabinLamp spix_device_flow.py)
yh_add_spix_fixture_test(HouseholdPicker spix_households.py)
yh_add_spix_fixture_test(Favorites spix_favorites.py)
yh_add_spix_fixture_test(ScenarioExecution spix_scenarios.py)
yh_add_spix_fixture_test(KeybindVisibility spix_settings.py)
yh_add_spix_fixture_test(FixtureSignIn spix_sign_in.py)

if(YH_TEST_NATIVE_TRAY)
  if(WIN32)
    add_executable(yh_tray_windows_desktop windows_desktop.cpp)
    target_link_libraries(yh_tray_windows_desktop PRIVATE Qt6::Gui user32)
    add_dependencies(appYandexHomeDesktop yh_tray_windows_desktop)
    yh_add_spix_fixture_test(TrayPlacement spix_tray_placement.py
      --windows-helper "$<TARGET_FILE:yh_tray_windows_desktop>")
  else()
    yh_add_spix_fixture_test(TrayPlacement spix_tray_placement.py)
  endif()
  set_tests_properties(TrayPlacement PROPERTIES RUN_SERIAL TRUE TIMEOUT 120)
endif()
