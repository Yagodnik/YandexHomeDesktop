include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

# Keep the existing module names both in-tree and in the installed SDK.
function(yh_shared_library target)
  string(TOLOWER "${target}" header_name)
  generate_export_header(${target}
    EXPORT_FILE_NAME "${PROJECT_BINARY_DIR}/include/yh/${header_name}_export.h")
  target_include_directories(${target} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/YandexHome>)
  target_compile_features(${target} PUBLIC cxx_std_23)
  set_target_properties(${target} PROPERTIES
    VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
    WINDOWS_EXPORT_ALL_SYMBOLS ON)
  add_library(YandexHome::${target} ALIAS ${target})
  install(TARGETS ${target} EXPORT YandexHomeTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT SDK
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT SDK
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT SDK)
  install(FILES "${PROJECT_BINARY_DIR}/include/yh/${header_name}_export.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/YandexHome/yh" COMPONENT SDK)
  set_property(GLOBAL APPEND PROPERTY YH_SDK_TARGETS ${target})
endfunction()

function(yh_install_sdk)
  # Model contracts use Hana in their public headers. Ship the pinned headers
  # with the SDK so an external consumer needs no checkout or network fetch.
  install(DIRECTORY "${hana_SOURCE_DIR}/include/boost"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/YandexHome" COMPONENT SDK)
  foreach(directory api services serialization cli)
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/${directory}/"
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/YandexHome/${directory}"
      COMPONENT SDK FILES_MATCHING PATTERN "*.h" PATTERN "debug" EXCLUDE)
  endforeach()
  install(FILES "${PROJECT_SOURCE_DIR}/src/auth/AuthResult.h"
    "${PROJECT_SOURCE_DIR}/src/auth/IAuthorizationFlow.h" "${PROJECT_SOURCE_DIR}/src/auth/ITokenStore.h"
    "${PROJECT_SOURCE_DIR}/src/auth/IAuthorizationService.h" "${PROJECT_SOURCE_DIR}/src/auth/AuthorizationService.h"
    "${PROJECT_SOURCE_DIR}/src/auth/AuthorizationFactory.h" "${PROJECT_SOURCE_DIR}/src/auth/KeychainTokenStore.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/YandexHome/auth" COMPONENT SDK)
  if(TARGET YandexOAuth)
    install(FILES "${PROJECT_SOURCE_DIR}/src/auth/OAuthConfiguration.h"
      "${PROJECT_SOURCE_DIR}/src/auth/QtOAuthAuthorizationFlow.h"
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/YandexHome/auth" COMPONENT SDK)
  endif()
  if(TARGET AppRest)
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/rest/"
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/YandexHome/rest"
      COMPONENT SDK FILES_MATCHING PATTERN "*.h")
  endif()
  install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/iot/core/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/YandexHome/iot/core"
    COMPONENT SDK FILES_MATCHING PATTERN "*.h")
  install(FILES "${PROJECT_SOURCE_DIR}/src/utils/Settings.h"
    "${PROJECT_SOURCE_DIR}/src/utils/LogManager.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/YandexHome/utils" COMPONENT SDK)
  get_property(YH_SDK_TARGETS GLOBAL PROPERTY YH_SDK_TARGETS)
  configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/YandexHomeConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/YandexHomeConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/YandexHome")
  write_basic_package_version_file("${PROJECT_BINARY_DIR}/YandexHomeConfigVersion.cmake"
    VERSION ${PROJECT_VERSION} COMPATIBILITY ExactVersion)
  install(EXPORT YandexHomeTargets NAMESPACE YandexHome::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/YandexHome" COMPONENT SDK)
  install(FILES "${PROJECT_BINARY_DIR}/YandexHomeConfig.cmake"
    "${PROJECT_BINARY_DIR}/YandexHomeConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/YandexHome" COMPONENT SDK)
endfunction()
