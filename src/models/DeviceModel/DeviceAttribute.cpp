#include "DeviceAttribute.h"

DeviceAttribute::DeviceAttribute(const CapabilityObject &source) {
  UpdateFrom(source);
}

DeviceAttribute::DeviceAttribute(const PropertyObject &source) {
  UpdateFrom(source);
}

void DeviceAttribute::PausePolling() {
  SetPending(true);
  SetPollingStartTime();
}

void DeviceAttribute::ResumePolling() {
  SetPending(false);
  SetPollingEndTime();
}

void DeviceAttribute::UpdateFrom(const CapabilityObject &source) {
  name = CapabilityType::operator[](source.type);
  state = source.state;
  parameters = source.parameters;
  type = Capability;
}

void DeviceAttribute::UpdateFrom(const PropertyObject &source) {
  name = PropertyType::operator[](source.type);
  state = source.state;
  parameters = source.parameters;
  type = Property;
}

void DeviceAttribute::SetPollingStartTime() {
  polling_start_time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;
}

void DeviceAttribute::SetPollingEndTime() {
  polling_finish_time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;
}

void DeviceAttribute::SetPending(const bool value) {
  is_pending = value;
}

void DeviceAttribute::SetPollingStartTime(const double time) {
  polling_start_time = time;
}

void DeviceAttribute::SetPollingEndTime(const double time) {
  polling_finish_time = time;
}

bool DeviceAttribute::IsInsideInterval(const double time, const double delta) const {
  return time >= (polling_start_time - delta)
        && time < (polling_finish_time + delta);
}
