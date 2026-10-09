add_library(AppRuntime SHARED Settings.cpp Settings.h LogManager.cpp LogManager.h)
target_link_libraries(AppRuntime PUBLIC Qt6::Core)
yh_shared_library(AppRuntime)

if(BUILD_DESKTOP_APP OR BUILD_TESTING)
  cmake_minimum_required(VERSION 3.16)

  find_package(Qt6 COMPONENTS Core Gui REQUIRED)

  add_library(Utils STATIC
          Themes.cpp
          Themes.h
          ErrorCodes.cpp
          ErrorCodes.h
          TitlesProvider.cpp
          TitlesProvider.h
          UnitsList.cpp
          UnitsList.h
          IconsProvider.cpp
          IconsProvider.h
          JsonLoader.cpp
          JsonLoader.h
          JsonLoaderRaw.h
  )

  target_link_libraries(Utils PRIVATE
          AppRuntime Qt6::Core Qt6::Gui
  )

endif()
