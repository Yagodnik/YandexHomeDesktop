cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Network NetworkAuth Gui REQUIRED)

add_library(YandexAuth STATIC
        AuthorizationService.cpp AuthorizationService.h
)

target_include_directories(YandexAuth PUBLIC
        ${CMAKE_SOURCE_DIR}/libs/qtkeychain/include/)

target_link_libraries(YandexAuth PRIVATE
  Qt6::Core
  Qt6::Network
  Qt6::NetworkAuth
  Qt6::Gui
)

if (APPLE)
  target_link_libraries(YandexAuth PUBLIC
          ${CMAKE_SOURCE_DIR}/libs/qtkeychain/lib/libqt6keychain.dylib)
elseif (UNIX)
  target_link_libraries(YandexAuth PUBLIC
          ${CMAKE_SOURCE_DIR}/libs/qtkeychain/lib/libqt6keychain.so)
elseif (WIN32)
  target_link_libraries(YandexAuth PUBLIC
          ${CMAKE_SOURCE_DIR}/libs/qtkeychain/lib/libqt6keychain.dll)
endif()

