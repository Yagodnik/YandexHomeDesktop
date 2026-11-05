yh_add_test(NAME SpixSmoke COMMAND "${Python3_EXECUTABLE}"
  "${CMAKE_CURRENT_SOURCE_DIR}/spix_smoke.py" "$<TARGET_FILE:appYandexHomeDesktop>")
yh_add_test(NAME CabinLamp CONFIGURATIONS Debug COMMAND "${Python3_EXECUTABLE}"
  "${CMAKE_CURRENT_SOURCE_DIR}/spix_device_flow.py" "$<TARGET_FILE:appYandexHomeDesktop>")
set_tests_properties(SpixSmoke CabinLamp PROPERTIES RESOURCE_LOCK spix_port_9000 TIMEOUT 90)
