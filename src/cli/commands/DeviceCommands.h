#pragma once
#include "cli/CommandRegistry.h"
#include "cli/CapabilityCommands.h"

class DeviceCommands { Q_DECLARE_TR_FUNCTIONS(DeviceCommands) };
void RegisterDeviceCommands(CommandRegistry& registry,
  const CapabilityCommands& capabilities = CapabilityCommands::Builtin());
