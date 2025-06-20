cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Network REQUIRED)

add_library(Models SHARED
        ScenariosModel.cpp ScenariosModel.h
        DevicesModel.cpp
        DevicesModel.h
        DevicesFilterModel.cpp
        DevicesFilterModel.h
        RoomsModel.cpp
        RoomsModel.h
        RoomsFilterModel.cpp
        RoomsFilterModel.h
)

target_link_libraries(Models PRIVATE Qt6::Core Qt6::Network YandexHomeApi)
