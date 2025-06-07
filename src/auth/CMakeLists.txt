cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Network NetworkAuth REQUIRED)

add_library(YandexAuth SHARED
  YandexAuth.cpp YandexAuth.h
)

target_link_libraries(YandexAuth PRIVATE Qt6::Core Qt6::Network Qt6::NetworkAuth)
