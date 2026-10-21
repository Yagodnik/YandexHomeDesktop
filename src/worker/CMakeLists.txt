add_library(WorkerControl SHARED
  WorkerControlClient.h WorkerControlClient.cpp
  WorkerControlServer.h WorkerControlServer.cpp
  WorkerControlPrivate.h WorkerControlPrivate.cpp)
target_link_libraries(WorkerControl PUBLIC Qt6::Core Qt6::Network)
yh_shared_library(WorkerControl)
