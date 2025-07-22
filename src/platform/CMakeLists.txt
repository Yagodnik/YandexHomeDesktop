find_package(Qt6 COMPONENTS Core Quick REQUIRED)

set(PLATFORM_SPECIFIC_SRC "")
set(PLATFORM_SPECIFIC_FRAMEWORKS "")

if(APPLE)
  set(PLATFORM_SPECIFIC_SRC macos/MacOSIntegration.mm)
  set_source_files_properties(macos/MacOSIntegration.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set(PLATFORM_SPECIFIC_FRAMEWORKS "-framework Cocoa")

  add_library(PlatformService
          PlatformService.cpp
          PlatformServiceMac.cpp
          ${PLATFORM_SPECIFIC_SRC}
  )
endif()

if(WIN32)
  add_library(PlatformService
          PlatformService.cpp
          PlatformServiceWin.cpp
  )
endif()


target_link_libraries(PlatformService PRIVATE Qt6::Core Qt6::Quick ${PLATFORM_SPECIFIC_FRAMEWORKS})
