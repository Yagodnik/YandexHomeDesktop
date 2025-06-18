find_package(Qt6 COMPONENTS Core REQUIRED)

set(MAC_SOURCES "")
set(MAC_FRAMEWORKS "")
if(APPLE)
  set(MAC_SOURCES macos/MacOSIntegration.mm)
  set_source_files_properties(macos/MacOSIntegration.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set(MAC_FRAMEWORKS "-framework Cocoa")
endif()

add_library(PlatformService
        PlatformService.cpp
        PlatformServiceMac.cpp
        ${MAC_SOURCES}
)

target_link_libraries(PlatformService PRIVATE Qt6::Core ${MAC_FRAMEWORKS})
