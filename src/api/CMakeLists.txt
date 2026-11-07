cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Network REQUIRED)

add_library(ApiContracts INTERFACE)
target_include_directories(ApiContracts INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${hana_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/YandexHome>)
target_link_libraries(ApiContracts INTERFACE Qt6::Core)

add_library(YandexApiAdapter SHARED
        YandexHomeApi.cpp
        YandexHomeApi.h
        YandexAccountApi.cpp
        YandexAccountApi.h
        QtHttpTransport.cpp
        QtHttpTransport.h
        RequestFactory.cpp
        model/Properties.h
        model/Serialization.cpp
)

target_link_libraries(YandexApiAdapter PUBLIC ApiContracts Qt6::Network)

add_library(FixtureApi STATIC EXCLUDE_FROM_ALL debug/FixtureApi.cpp debug/FixtureApi.h)
target_link_libraries(FixtureApi PUBLIC ApiContracts PRIVATE YandexApiAdapter)

yh_shared_library(YandexApiAdapter)

add_library(YandexHome::ApiContracts ALIAS ApiContracts)
install(TARGETS ApiContracts EXPORT YandexHomeTargets COMPONENT SDK)
