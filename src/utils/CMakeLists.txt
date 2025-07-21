cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Gui REQUIRED)

add_library(Utils STATIC
        Themes.cpp
        Themes.h
        ErrorCodes.cpp
        ErrorCodes.h
        Settings.cpp
        Settings.h
        TitlesProvider.cpp
        TitlesProvider.h
        UnitsList.cpp
        UnitsList.h
        IconsProvider.cpp
        IconsProvider.h
        JsonLoader.cpp
        JsonLoader.h
        LogManager.cpp
        LogManager.h
)

target_link_libraries(Utils PRIVATE
        Qt6::Core Qt6::Gui
)
