#include "CapabilityRules.h"

namespace Iot::Rules {

std::shared_ptr<const ICapabilityRules> ForType(CapabilityType type) {
  if (type == CapabilityType::OnOff) {
    return OnOff();
  }
  if (type == CapabilityType::Range) {
    return Range();
  }
  if (type == CapabilityType::Toggle) {
    return Toggle();
  }
  if (type == CapabilityType::Mode) {
    return Mode();
  }
  if (type == CapabilityType::ColorSetting) {
    return Color();
  }
  return {};
}

} // namespace Iot::Rules
