find_package(Qt6 COMPONENTS Core Network Test REQUIRED)

add_executable(AppTests
        SerializationTests.cpp
        ApiTests.cpp
        TestsMain.cpp
        SerializationTests.h
)

target_link_libraries(AppTests PRIVATE Qt6::Core Qt6::Network Qt6::Test YandexHomeApi)

add_test(NAME All COMMAND AppTests)
