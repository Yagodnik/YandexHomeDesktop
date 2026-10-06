#pragma once
#include "cli/CapabilityCommands.h"
#include "cli/CommandRegistry.h"

class DeviceCommands {
  Q_DECLARE_TR_FUNCTIONS(DeviceCommands)
};
void RegisterDeviceCommands(CommandRegistry& registry,
                            const CapabilityCommands& capabilities = CapabilityCommands::Builtin());
