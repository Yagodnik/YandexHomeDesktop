add_library(AuthContracts SHARED
  AuthResult.h ITokenStore.h IAuthorizationFlow.h IAuthorizationService.h)
target_link_libraries(AuthContracts PUBLIC Qt6::Core)

add_library(AuthCore SHARED AuthorizationService.cpp AuthorizationService.h)
target_link_libraries(AuthCore PUBLIC AuthContracts)

add_library(YandexAuth SHARED
  AuthorizationFactory.cpp AuthorizationFactory.h KeychainTokenStore.cpp KeychainTokenStore.h
  "$<$<CONFIG:Debug>:debug/FixtureAuthorizationService.cpp>"
  "$<$<CONFIG:Debug>:debug/FixtureAuthorizationService.h>")
target_compile_definitions(YandexAuth PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
target_link_libraries(YandexAuth PUBLIC AuthCore PRIVATE qt6keychain Qt6::Network)
yh_shared_library(YandexAuth)

if(BUILD_DESKTOP_APP)
  add_library(YandexOAuth SHARED OAuthConfiguration.cpp OAuthConfiguration.h
    QtOAuthAuthorizationFlow.cpp QtOAuthAuthorizationFlow.h)
  target_link_libraries(YandexOAuth PUBLIC AuthCore Qt6::NetworkAuth PRIVATE Qt6::Gui)
  yh_shared_library(YandexOAuth)
endif()

yh_shared_library(AuthContracts)

yh_shared_library(AuthCore)
