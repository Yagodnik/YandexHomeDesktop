#pragma once
#include "cli/CapabilityCommands.h"

namespace CliCapability {
std::shared_ptr<const ICapabilityInput> OnOff();
std::shared_ptr<const ICapabilityInput> Toggle();
std::shared_ptr<const ICapabilityInput> Range();
std::shared_ptr<const ICapabilityInput> Mode();
std::shared_ptr<const ICapabilityInput> Color();
} // namespace CliCapability
