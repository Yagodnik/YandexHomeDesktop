#pragma once
#include "app/common/StartupOptions.h"
#include "worker/WorkerControlClient.h"

struct McpConfiguration {
  QString gui_endpoint;
  WorkerLaunchConfiguration worker;
};
McpConfiguration MakeMcpConfiguration(const StartupOptions& options);
