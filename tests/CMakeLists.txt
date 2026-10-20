find_package(Qt6 COMPONENTS Core Network Test Gui REQUIRED)

foreach(_suite Serialization Api ApiBoundary ModelBoundary Capabilities Models)
  add_executable(${_suite}Tests ${_suite}Tests.cpp ${_suite}Tests.h QtTestMain.cpp)
  target_compile_definitions(${_suite}Tests PRIVATE
    YH_TEST_HEADER="${_suite}Tests.h" YH_TEST_CLASS=${_suite}Tests)
  target_link_libraries(${_suite}Tests PRIVATE
    Qt6::Core Qt6::Network Qt6::Test Qt6::Gui YandexApiAdapter Iot Utils Models)
  yh_add_test(NAME ${_suite} COMMAND ${_suite}Tests)
endforeach()

yh_add_test(NAME TestTooling COMMAND "${Python3_EXECUTABLE}"
  "${CMAKE_CURRENT_SOURCE_DIR}/TestCatalogToolTests.py")
yh_add_test(NAME TestCatalog COMMAND "${Python3_EXECUTABLE}"
  "${PROJECT_SOURCE_DIR}/scripts/test_catalog.py" check
  --build-dir "${PROJECT_BINARY_DIR}" --config "$<CONFIG>" --ctest "${CMAKE_CTEST_COMMAND}")
set_tests_properties(TestCatalog PROPERTIES TIMEOUT 90)

if(BUILD_SHORTCUTS_APP)
  add_executable(NativeShortcutTests NativeShortcutTests.cpp ../src/app/shortcuts/QHotkeyRegistrar.cpp)
  target_link_libraries(NativeShortcutTests PRIVATE Qt6::Test QHotkey::QHotkey AppShortcuts)
  yh_add_test(NAME NativeShortcuts COMMAND NativeShortcutTests)
  set_tests_properties(NativeShortcuts PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
  add_executable(ShortcutProcessTests ShortcutProcessTests.cpp)
  target_link_libraries(ShortcutProcessTests PRIVATE Qt6::Test ShortcutControlBootstrap)
  target_compile_definitions(ShortcutProcessTests PRIVATE FIXTURE_PATH="${CMAKE_SOURCE_DIR}/resources/debug/api.json")
  add_dependencies(ShortcutProcessTests YandexHomeShortcuts)
  yh_add_test(NAME ShortcutProcess COMMAND ShortcutProcessTests $<TARGET_FILE:YandexHomeShortcuts> "$<IF:$<CONFIG:Debug>,1,0>")
  set_tests_properties(ShortcutProcess PROPERTIES TIMEOUT 90 ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

add_executable(ShortcutTests ShortcutTests.cpp)
target_link_libraries(ShortcutTests PRIVATE Qt6::Test AppShortcuts)
yh_add_test(NAME Shortcuts COMMAND ShortcutTests)

add_executable(WorkerControlTests WorkerControlTests.cpp)
target_link_libraries(WorkerControlTests PRIVATE Qt6::Test WorkerControl)
yh_add_test(NAME WorkerControl COMMAND WorkerControlTests
  -o -,txt -o "${PROJECT_BINARY_DIR}/Testing/WorkerControl.txt,txt")
set_tests_properties(WorkerControl PROPERTIES TIMEOUT 90)

add_executable(AuthorizationTests AuthorizationTests.cpp)
target_link_libraries(AuthorizationTests PRIVATE Qt6::Test AuthCore Models)
yh_add_test(NAME Authorization COMMAND AuthorizationTests)

if(BUILD_DESKTOP_APP)
  add_executable(OAuthAdapterTests OAuthAdapterTests.cpp)
  target_link_libraries(OAuthAdapterTests PRIVATE Qt6::Test YandexAuth YandexOAuth)
  yh_add_test(NAME OAuthAdapter COMMAND OAuthAdapterTests)
endif()

add_executable(DeviceControlTests DeviceControlTests.cpp)
target_link_libraries(DeviceControlTests PRIVATE Qt6::Test Models)
yh_add_test(NAME DeviceControl COMMAND DeviceControlTests)

add_executable(DeviceServiceTests DeviceServiceTests.cpp)
target_link_libraries(DeviceServiceTests PRIVATE Qt6::Test AppServices)
yh_add_test(NAME DeviceService COMMAND DeviceServiceTests)

add_executable(HomeTests HomeTests.cpp)
target_link_libraries(HomeTests PRIVATE Qt6::Test Models)
yh_add_test(NAME Home COMMAND HomeTests)

add_executable(ApiFixtureTests ApiFixtureTests.cpp)
qt_add_resources(API_FIXTURE_RESOURCES "${PROJECT_SOURCE_DIR}/resources/debug/debug.qrc")
target_sources(ApiFixtureTests PRIVATE ${API_FIXTURE_RESOURCES})
target_link_libraries(ApiFixtureTests PRIVATE Qt6::Test FixtureApi AppStartup Utils)
target_compile_definitions(ApiFixtureTests PRIVATE FIXTURE_PATH="${CMAKE_SOURCE_DIR}/resources/debug/api.json")
yh_add_test(NAME ApiFixture COMMAND ApiFixtureTests)

add_executable(KeybindSettingsTests KeybindSettingsTests.cpp)
target_link_libraries(KeybindSettingsTests PRIVATE Qt6::Test Models)
yh_add_test(NAME KeybindSettings COMMAND KeybindSettingsTests)

add_executable(SettingsTransferTests SettingsTransferTests.cpp)
target_link_libraries(SettingsTransferTests PRIVATE Qt6::Test Models)
yh_add_test(NAME SettingsTransfer COMMAND SettingsTransferTests)

add_executable(ScenariosTests ScenariosTests.cpp)
target_link_libraries(ScenariosTests PRIVATE Qt6::Test Models)
yh_add_test(NAME Scenarios COMMAND ScenariosTests)
if(BUILD_DESKTOP_APP)
  set_tests_properties(Scenarios Home PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_QUICK_CONTROLS_STYLE=Basic;QT_QPA_FONTDIR=${PROJECT_SOURCE_DIR}/resources/fonts")
endif()

add_executable(CliTests CliTests.cpp)
target_link_libraries(CliTests PRIVATE Qt6::Test AppCli AppStartup)
yh_add_test(NAME Cli COMMAND CliTests)

add_executable(CapabilityRulesTests CapabilityRulesTests.cpp)
target_link_libraries(CapabilityRulesTests PRIVATE Qt6::Test IotCore)
yh_add_test(NAME CapabilityRules COMMAND CapabilityRulesTests)

if(YH_BUILD_REST_ADAPTER)
add_executable(RestTests RestTests.cpp)
target_link_libraries(RestTests PRIVATE Qt6::Test AppRest AppRestControl AppStartup RestControlBootstrap Models)
target_compile_definitions(RestTests PRIVATE OPENAPI_PATH="${PROJECT_SOURCE_DIR}/resources/rest/openapi.json")
yh_add_test(NAME Rest COMMAND RestTests)

endif()

if(BUILD_CLI_APP AND BUILD_REST_APP)
  find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
  add_executable(RestProcessTests RestProcessTests.cpp)
  target_link_libraries(RestProcessTests PRIVATE Qt6::Test Qt6::Network)
  if(APPLE)
    target_link_libraries(RestProcessTests PRIVATE "-framework CoreFoundation")
  endif()
  target_compile_definitions(RestProcessTests PRIVATE FIXTURE_PATH="${CMAKE_SOURCE_DIR}/resources/debug/api.json")
  target_compile_definitions(RestProcessTests PRIVATE PYTHON_PATH="${Python3_EXECUTABLE}"
    EXAMPLE_PATH="${CMAKE_SOURCE_DIR}/examples/rest_client.py")
endif()

if(BUILD_CLI_APP)
  yh_add_test(NAME CliProcess_YandexHomeCli COMMAND "${CMAKE_COMMAND}"
    "-DAPP_PATH=$<TARGET_FILE:YandexHomeCli>" "-DCONFIGURATION=$<CONFIG>"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/CliSmoke.cmake")
endif()
if(BUILD_CLI_APP AND BUILD_REST_APP)
  yh_add_test(NAME RestProcess COMMAND RestProcessTests
    "$<TARGET_FILE:YandexHomeCli>" "$<TARGET_FILE:YandexHomeRest>" "$<CONFIG>")
  set_tests_properties(RestProcess PROPERTIES TIMEOUT 90)
endif()
if(BUILD_DESKTOP_APP)
  yh_add_test(NAME GuiRejectsCommands COMMAND "${CMAKE_COMMAND}"
    "-DAPP_PATH=$<TARGET_FILE:appYandexHomeDesktop>"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/GuiEntryPoint.cmake")
endif()

yh_add_test(NAME InstalledSdkConsumer COMMAND "${CMAKE_COMMAND}"
  "-DBUILD_PATH=${PROJECT_BINARY_DIR}" "-DSOURCE_PATH=${PROJECT_SOURCE_DIR}"
  "-DCONFIGURATION=$<CONFIG>" "-DGENERATOR=${CMAKE_GENERATOR}"
  "-DCOMPILER=${CMAKE_CXX_COMPILER}" "-DQT_PACKAGE_PATH=${Qt6_DIR}"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/SdkConsumer.cmake")
set_tests_properties(InstalledSdkConsumer PROPERTIES TIMEOUT 90)
