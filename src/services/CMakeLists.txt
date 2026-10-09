add_library(AppServices SHARED
        ScenarioService.h
        ScenarioService.cpp
        HomeService.h
        HomeService.cpp
        DeviceService.h
        DeviceService.cpp
        DeviceSession.h
        DeviceSession.cpp
        AccountService.h
        CommandResult.h
        ApiJson.h ApiJson.cpp
)

target_link_libraries(AppServices PUBLIC ApiContracts IotCore Qt6::Core)

add_library(AppRestControl SHARED RestControlService.h RestControlService.cpp)
target_link_libraries(AppRestControl PUBLIC Qt6::Network AppRuntime)

yh_shared_library(AppServices)

yh_shared_library(AppRestControl)
