yh_add_test(NAME SpixSmoke COMMAND "${Python3_EXECUTABLE}"
  "${CMAKE_CURRENT_SOURCE_DIR}/spix_smoke.py" "$<TARGET_FILE:appYandexHomeDesktop>")
set_tests_properties(SpixSmoke PROPERTIES RESOURCE_LOCK spix_port_9000 TIMEOUT 90)

function(yh_add_spix_fixture_test name script)
  yh_add_test(NAME "${name}" CONFIGURATIONS Debug COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${script}" "$<TARGET_FILE:appYandexHomeDesktop>")
  set_tests_properties("${name}" PROPERTIES RESOURCE_LOCK spix_port_9000 TIMEOUT 90)
endfunction()

yh_add_spix_fixture_test(CabinLamp spix_device_flow.py)
yh_add_spix_fixture_test(HouseholdPicker spix_households.py)
yh_add_spix_fixture_test(Favorites spix_favorites.py)
yh_add_spix_fixture_test(ScenarioExecution spix_scenarios.py)
yh_add_spix_fixture_test(KeybindVisibility spix_settings.py)
yh_add_spix_fixture_test(FixtureSignIn spix_sign_in.py)
