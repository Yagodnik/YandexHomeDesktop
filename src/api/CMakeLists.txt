cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(YandexHomeApi SHARED
  YandexHomeApi.cpp YandexHomeApi.h
)

target_link_libraries(YandexHomeApi PRIVATE Qt6::Core Qt6::Network)
