add_library(AppRest SHARED
        RestServer.h RestServer.cpp
        RestProtocol.h
        RestReply.h RestReply.cpp
        RestRouter.h RestRouter.cpp
        RestEndpoints.h RestEndpoints.cpp
        RestAction.h RestAction.cpp
        RestErrors.h RestErrors.cpp
)
target_link_libraries(AppRest PUBLIC AppServices Qt6::HttpServer Qt6::Network)

qt_add_resources(AppRest "rest_openapi"
  PREFIX "/rest"
  BASE "${PROJECT_SOURCE_DIR}/resources/rest"
  FILES "${PROJECT_SOURCE_DIR}/resources/rest/openapi.json")

install(FILES "${PROJECT_SOURCE_DIR}/resources/rest/openapi.json"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/YandexHome/rest" COMPONENT SDK)

yh_shared_library(AppRest)
