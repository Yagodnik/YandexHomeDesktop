cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Network Gui REQUIRED)

add_library(ApiContracts INTERFACE)
target_include_directories(ApiContracts INTERFACE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/libs)
target_link_libraries(ApiContracts INTERFACE Qt6::Core)

add_library(YandexApiAdapter STATIC
        YandexHomeApi.cpp
        YandexHomeApi.h
        YandexAccountApi.cpp
        YandexAccountApi.h
        QtHttpTransport.cpp
        QtHttpTransport.h
        RequestFactory.cpp
        model/Properties.h
)

target_link_libraries(YandexApiAdapter PUBLIC ApiContracts Qt6::Network PRIVATE Qt6::Gui)
