cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Gui Network REQUIRED)

add_library(Backend STATIC
        RoomsBackend.cpp
        RoomsBackend.h
)

target_link_libraries(Backend PRIVATE Qt6::Core Qt6::Network Qt6::Gui YandexHomeApi Utils)
