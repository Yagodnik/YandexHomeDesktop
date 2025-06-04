find_package(Qt6 COMPONENTS Core Test REQUIRED)

add_executable(AppTests
    SerializationTest.cpp
)

target_link_libraries(AppTests PRIVATE Qt6::Core Qt6::Test)


add_test(NAME All COMMAND AppTests)
