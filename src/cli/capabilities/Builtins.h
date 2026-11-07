#pragma once

#include "yh/appcli_export.h"
#include "cli/CapabilityCommands.h"

namespace CliCapability {
APPCLI_EXPORT std::shared_ptr<const ICapabilityInput> OnOff();
APPCLI_EXPORT std::shared_ptr<const ICapabilityInput> Toggle();
APPCLI_EXPORT std::shared_ptr<const ICapabilityInput> Range();
APPCLI_EXPORT std::shared_ptr<const ICapabilityInput> Mode();
APPCLI_EXPORT std::shared_ptr<const ICapabilityInput> Color();
} // namespace CliCapability
