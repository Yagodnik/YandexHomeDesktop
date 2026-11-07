#pragma once

#include "yh/appcli_export.h"
#include "cli/CapabilityCommands.h"
#include "cli/CommandRegistry.h"

class DeviceCommands {
  Q_DECLARE_TR_FUNCTIONS(DeviceCommands)
};
APPCLI_EXPORT void RegisterDeviceCommands(CommandRegistry& registry,
                            const CapabilityCommands& capabilities = CapabilityCommands::Builtin());
