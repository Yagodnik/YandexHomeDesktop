#include "CliOutput.h"
#include <QJsonArray>

QJsonObject CapabilityJson(const CapabilityObject& capability) {
  return {{"type", CapabilityType::AsString(capability.type)},
          {"retrievable", capability.retrievable},
          {"parameters", QJsonObject::fromVariantMap(capability.parameters)},
          {"state", QJsonObject::fromVariantMap(capability.state)},
          {"last_updated", capability.last_updated}};
}

QJsonObject DeviceJson(const DeviceInfo& device) {
  QJsonArray capabilities, properties;
  for (const auto& capability : device.capabilities) {
    capabilities.append(CapabilityJson(capability));
  }
  for (const auto& property : device.properties) {
    properties.append(QJsonObject{{"type", PropertyType::AsString(property.type)},
                                  {"retrievable", property.retrievable},
                                  {"state", QJsonObject::fromVariantMap(property.state)},
                                  {"parameters", QJsonObject::fromVariantMap(property.parameters)},
                                  {"last_updated", property.last_updated}});
  }
  return {{"id", device.id},
          {"name", device.name},
          {"type", device.type},
          {"state", DeviceState::AsString(device.state)},
          {"capabilities", capabilities},
          {"properties", properties}};
}
