add_library(AuthContracts STATIC
  AuthResult.h ITokenStore.h IAuthorizationFlow.h IAuthorizationService.h)
target_link_libraries(AuthContracts PUBLIC Qt6::Core)

add_library(AuthCore STATIC AuthorizationService.cpp AuthorizationService.h)
target_link_libraries(AuthCore PUBLIC AuthContracts)

if(BUILD_DESKTOP_APP)
  add_library(YandexAuth STATIC
    AuthorizationFactory.cpp AuthorizationFactory.h
    KeychainTokenStore.cpp KeychainTokenStore.h
    OAuthConfiguration.cpp OAuthConfiguration.h
    QtOAuthAuthorizationFlow.cpp QtOAuthAuthorizationFlow.h
    "$<$<CONFIG:Debug>:debug/FixtureAuthorizationService.cpp>"
    "$<$<CONFIG:Debug>:debug/FixtureAuthorizationService.h>"
  )
  target_compile_definitions(YandexAuth PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
  target_link_libraries(YandexAuth PUBLIC AuthCore PRIVATE
    qt6keychain Qt6::Network Qt6::NetworkAuth Qt6::Gui)
endif()
