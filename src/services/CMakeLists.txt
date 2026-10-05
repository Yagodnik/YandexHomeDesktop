add_library(AppServices STATIC
        ScenarioService.h
        ScenarioService.cpp
        HomeService.h
        HomeService.cpp
)

target_link_libraries(AppServices PUBLIC ApiContracts Qt6::Core)
