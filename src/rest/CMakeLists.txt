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

yh_shared_library(AppRest)
