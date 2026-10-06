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
