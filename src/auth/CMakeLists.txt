cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Network NetworkAuth Gui REQUIRED)

add_library(YandexAuth STATIC
        AuthorizationService.cpp AuthorizationService.h
)

target_link_libraries(YandexAuth PUBLIC
  qt6keychain
  Qt6::Core
  Qt6::Network
  Qt6::NetworkAuth
  Qt6::Gui
)
