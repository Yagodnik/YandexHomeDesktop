find_package(Qt6 COMPONENTS Core Network Test Gui REQUIRED)

add_executable(AppTests
        ApiBoundaryTests.cpp
        ApiBoundaryTests.h
        ModelBoundaryTests.cpp
        ModelBoundaryTests.h
        SerializationTests.cpp
        ApiTests.cpp
        TestsMain.cpp
        SerializationTests.h
        CapabilitiesTests.cpp
        CapabilitiesTests.h
        ModelsTests.cpp
        ModelsTests.h
)

target_link_libraries(AppTests PRIVATE Qt6::Core Qt6::Network Qt6::Test Qt6::Gui YandexApiAdapter Iot Utils Models)

add_test(NAME All COMMAND AppTests)

add_executable(AuthorizationTests AuthorizationTests.cpp)
target_link_libraries(AuthorizationTests PRIVATE Qt6::Test AuthCore Models)
add_test(NAME Authorization COMMAND AuthorizationTests)
set_tests_properties(Authorization PROPERTIES TIMEOUT 30)

if(BUILD_DESKTOP_APP)
  add_executable(OAuthAdapterTests OAuthAdapterTests.cpp)
  target_link_libraries(OAuthAdapterTests PRIVATE Qt6::Test YandexAuth YandexOAuth)
  add_test(NAME OAuthAdapter COMMAND OAuthAdapterTests)
  set_tests_properties(OAuthAdapter PROPERTIES TIMEOUT 30)
endif()

add_executable(DeviceControlTests DeviceControlTests.cpp)
target_link_libraries(DeviceControlTests PRIVATE Qt6::Test Models)
add_test(NAME DeviceControl COMMAND DeviceControlTests)
set_tests_properties(DeviceControl PROPERTIES TIMEOUT 30)

add_executable(DeviceServiceTests DeviceServiceTests.cpp)
target_link_libraries(DeviceServiceTests PRIVATE Qt6::Test AppServices)
add_test(NAME DeviceService COMMAND DeviceServiceTests)
set_tests_properties(DeviceService PROPERTIES TIMEOUT 30)

add_executable(HomeTests HomeTests.cpp)
target_link_libraries(HomeTests PRIVATE Qt6::Test Models)
add_test(NAME Home COMMAND HomeTests)
set_tests_properties(Home PROPERTIES TIMEOUT 30)

add_executable(ApiFixtureTests ApiFixtureTests.cpp)
target_link_libraries(ApiFixtureTests PRIVATE Qt6::Test FixtureApi AppStartup Utils)
target_compile_definitions(ApiFixtureTests PRIVATE FIXTURE_PATH="${CMAKE_SOURCE_DIR}/resources/debug/api.json")
add_test(NAME ApiFixture COMMAND ApiFixtureTests)
set_tests_properties(ApiFixture PROPERTIES TIMEOUT 30)

add_executable(ScenariosTests ScenariosTests.cpp)
target_link_libraries(ScenariosTests PRIVATE Qt6::Test Models)
add_test(NAME Scenarios COMMAND ScenariosTests)
set_tests_properties(Scenarios PROPERTIES TIMEOUT 30)
if(BUILD_DESKTOP_APP)
  set_tests_properties(Scenarios Home PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_QUICK_CONTROLS_STYLE=Basic")
endif()

add_executable(CliTests CliTests.cpp)
target_link_libraries(CliTests PRIVATE Qt6::Test AppCli AppStartup)
add_test(NAME Cli COMMAND CliTests)
set_tests_properties(Cli PROPERTIES TIMEOUT 30)

add_executable(CapabilityRulesTests CapabilityRulesTests.cpp)
target_link_libraries(CapabilityRulesTests PRIVATE Qt6::Test IotCore)
add_test(NAME CapabilityRules COMMAND CapabilityRulesTests)
set_tests_properties(CapabilityRules PROPERTIES TIMEOUT 30)

if(YH_BUILD_REST_ADAPTER)
add_executable(RestTests RestTests.cpp)
target_link_libraries(RestTests PRIVATE Qt6::Test AppRest AppRestControl AppStartup Models)
add_test(NAME Rest COMMAND RestTests)
set_tests_properties(Rest PROPERTIES TIMEOUT 30)

endif()

if(BUILD_CLI_APP AND BUILD_REST_APP)
  find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
  add_executable(RestProcessTests RestProcessTests.cpp)
  target_link_libraries(RestProcessTests PRIVATE Qt6::Test Qt6::Network)
  target_compile_definitions(RestProcessTests PRIVATE FIXTURE_PATH="${CMAKE_SOURCE_DIR}/resources/debug/api.json")
  target_compile_definitions(RestProcessTests PRIVATE PYTHON_PATH="${Python3_EXECUTABLE}"
    EXAMPLE_PATH="${CMAKE_SOURCE_DIR}/examples/rest_client.py")
endif()

if(BUILD_CLI_APP)
  add_test(NAME CliProcess_YandexHomeCli COMMAND "${CMAKE_COMMAND}"
    "-DAPP_PATH=$<TARGET_FILE:YandexHomeCli>" "-DCONFIGURATION=$<CONFIG>"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/CliSmoke.cmake")
  set_tests_properties(CliProcess_YandexHomeCli PROPERTIES TIMEOUT 30)
endif()
if(BUILD_CLI_APP AND BUILD_REST_APP)
  add_test(NAME RestProcess COMMAND RestProcessTests
    "$<TARGET_FILE:YandexHomeCli>" "$<TARGET_FILE:YandexHomeRest>" "$<CONFIG>")
  set_tests_properties(RestProcess PROPERTIES TIMEOUT 90)
endif()
if(BUILD_DESKTOP_APP)
  add_test(NAME GuiRejectsCommands COMMAND "${CMAKE_COMMAND}"
    "-DAPP_PATH=$<TARGET_FILE:appYandexHomeDesktop>"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/GuiEntryPoint.cmake")
endif()

add_test(NAME InstalledSdkConsumer COMMAND "${CMAKE_COMMAND}"
  "-DBUILD_PATH=${PROJECT_BINARY_DIR}" "-DSOURCE_PATH=${PROJECT_SOURCE_DIR}"
  "-DCONFIGURATION=$<CONFIG>" "-DGENERATOR=${CMAKE_GENERATOR}"
  "-DCOMPILER=${CMAKE_CXX_COMPILER}" "-DQT_PACKAGE_PATH=${Qt6_DIR}"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/SdkConsumer.cmake")
set_tests_properties(InstalledSdkConsumer PROPERTIES TIMEOUT 90)
