
find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(IotCore SHARED
        core/CapabilityState.h core/CapabilityState.cpp
        core/CapabilityParameters.h core/CapabilityParameters.cpp
        core/CapabilityRules.h core/CapabilityRules.cpp core/Validation.h core/Validation.cpp
        core/BooleanRules.cpp core/RangeRules.cpp core/ModeRules.cpp core/ColorRules.cpp
)
target_link_libraries(IotCore PUBLIC ApiContracts Qt6::Core)

if(BUILD_DESKTOP_APP OR BUILD_TESTING)
  add_library(Iot STATIC
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

  target_link_libraries(Iot PUBLIC IotCore PRIVATE Qt6::Core Qt6::Gui Utils)

endif()

yh_shared_library(IotCore)
