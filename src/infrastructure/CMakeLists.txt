
find_package(Qt6 REQUIRED COMPONENTS Core Gui)

add_library(Infrastructre STATIC
        settings/SettingsStorage.cpp
)

target_link_libraries(Iot PRIVATE Qt6::Core Qt6::Gui Utils)
