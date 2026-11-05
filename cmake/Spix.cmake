# Build the automation dependencies privately and statically in opt-in builds.
function(yh_link_spix target)
  FetchContent_Declare(anyrpc
    GIT_REPOSITORY https://github.com/sgieseking/anyrpc.git
    GIT_TAG b1949b3d40849229055ae75cf5334b9d08579502
    SOURCE_SUBDIR no-cmake-subdirectory)
  FetchContent_Declare(spix
    GIT_REPOSITORY https://github.com/faaxm/spix.git
    GIT_TAG 57cf7105e1250b999104d6439df7f449ba1afe93
    SOURCE_SUBDIR no-cmake-subdirectory)
  FetchContent_MakeAvailable(anyrpc spix)

  # AnyRPC's legacy build assumes it owns CMAKE_SOURCE_DIR and writes headers
  # into its source tree. Build only its XML-RPC transport and generate the
  # configuration header alongside the other build outputs.
  set(ANYRPC_VERSION_STRING "1.0.2")
  set(ANYRPC_VERSION_MAJOR 1)
  set(ANYRPC_VERSION_MINOR 0)
  set(ANYRPC_VERSION_PATCH 2)
  set(ANYRPC_INCLUDE_XML ON)
  set(ANYRPC_ASSERT_LEVEL 1)
  set(ANYRPC_THROW_LEVEL 1)
  configure_file("${anyrpc_SOURCE_DIR}/version.h.in" "${anyrpc_BINARY_DIR}/include/anyrpc/version.h")
  file(GLOB _anyrpc_sources
    "${anyrpc_SOURCE_DIR}/src/*.cpp"
    "${anyrpc_SOURCE_DIR}/src/internal/*.cpp"
    "${anyrpc_SOURCE_DIR}/src/xml/*.cpp")
  add_library(yh_anyrpc STATIC ${_anyrpc_sources})
  set_target_properties(yh_anyrpc PROPERTIES CXX_STANDARD 11 AUTOMOC OFF AUTORCC OFF)
  target_include_directories(yh_anyrpc SYSTEM PUBLIC
    "${anyrpc_BINARY_DIR}/include/anyrpc" "${anyrpc_SOURCE_DIR}/include")
  if(WIN32)
    target_compile_definitions(yh_anyrpc PUBLIC WIN32)
    target_link_libraries(yh_anyrpc PUBLIC ws2_32)
  endif()
  if(MINGW)
    target_compile_options(yh_anyrpc PRIVATE -U__STRICT_ANSI__)
  endif()
  add_library(AnyRPC::anyrpc INTERFACE IMPORTED)
  target_link_libraries(AnyRPC::anyrpc INTERFACE yh_anyrpc)
  # Spix's FindAnyRPC validates these variables and reuses the existing target.
  set(AnyRPC_INCLUDE_DIRS "${anyrpc_SOURCE_DIR}/include")
  set(AnyRPC_LIBRARIES AnyRPC::anyrpc)

  # Apply the loopback and shutdown fixes to a build-tree copy, including when
  # the original sources come from an offline FetchContent override.
  find_package(Git REQUIRED)
  set(_patched "${spix_BINARY_DIR}/patched")
  # COPY can skip patched files whose timestamps match the originals.
  file(REMOVE_RECURSE "${_patched}")
  file(MAKE_DIRECTORY "${_patched}")
  file(COPY "${spix_SOURCE_DIR}/" DESTINATION "${_patched}" PATTERN ".git" EXCLUDE)
  set(_patch "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Spix.patch")
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_patch}")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" --git-dir=${_patched}/.unused-git-dir
      apply --no-index --ignore-space-change "${_patch}"
    WORKING_DIRECTORY "${_patched}"
    COMMAND_ERROR_IS_FATAL ANY)

  set(BUILD_SHARED_LIBS OFF)
  set(CMAKE_AUTOMOC OFF) # Spix generates its own moc files.
  set(SPIX_BUILD_EXAMPLES OFF)
  set(SPIX_BUILD_TESTS OFF)
  set(SPIX_BUILD_QTQUICK ON)
  set(SPIX_BUILD_QTWIDGETS OFF)
  set(SPIX_QT_MAJOR 6)
  add_subdirectory("${_patched}" "${spix_BINARY_DIR}/build" EXCLUDE_FROM_ALL)
  target_link_libraries(${target} PRIVATE Spix::QtQuick)
  target_compile_definitions(${target} PRIVATE YH_ENABLE_SPIX)
  install(FILES "${spix_SOURCE_DIR}/LICENSE.txt"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/YandexHome/licenses/Spix")
  install(FILES "${anyrpc_SOURCE_DIR}/license"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/YandexHome/licenses/AnyRPC")
  if(APPLE)
    install(FILES "${spix_SOURCE_DIR}/LICENSE.txt"
      DESTINATION "YandexHomeDesktop.app/Contents/Resources/licenses/Spix" COMPONENT DesktopBundle)
    install(FILES "${anyrpc_SOURCE_DIR}/license"
      DESTINATION "YandexHomeDesktop.app/Contents/Resources/licenses/AnyRPC" COMPONENT DesktopBundle)
  endif()
endfunction()
