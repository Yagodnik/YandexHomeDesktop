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
