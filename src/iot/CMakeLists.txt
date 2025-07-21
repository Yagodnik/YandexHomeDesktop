
find_package(Qt6 REQUIRED COMPONENTS Core Gui)

add_library(Iot
        capabilities/OnOffCapability.cpp
        capabilities/OnOffCapability.h
        capabilities/RangeCapability.cpp
        capabilities/RangeCapability.h
        common/IotObject.cpp
        common/IotObject.h
        capabilities/ToggleCapability.cpp
        capabilities/ToggleCapability.h
        capabilities/ColorSettingCapability.cpp
        capabilities/ColorSettingCapability.h

        capabilities/ModesCapability.cpp
        capabilities/ModesCapability.h

        properties/FloatProperty.cpp
        properties/FloatProperty.h
        properties/EventProperty.cpp
        properties/EventProperty.h
)

target_link_libraries(Iot PRIVATE Qt6::Core Qt6::Gui Utils)
