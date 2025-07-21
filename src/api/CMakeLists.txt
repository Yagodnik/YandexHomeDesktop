cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Network Gui REQUIRED)

add_library(YandexHomeApi STATIC
        YandexHomeApi.cpp
        YandexHomeApi.h
        YandexAccount.cpp
        YandexAccount.h
        RequestFactory.cpp
        model/Properties.h
)

target_link_libraries(YandexHomeApi PRIVATE Qt6::Core Qt6::Network Qt6::Gui)
