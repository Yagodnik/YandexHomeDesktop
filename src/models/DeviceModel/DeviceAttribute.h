#pragma once

#include <QVariant>

#include "api/model/Capabilites.h"
#include "api/model/Properties.h"

struct DeviceAttribute {
  DeviceAttribute() = default;

  explicit DeviceAttribute(const CapabilityObject& source);
  explicit DeviceAttribute(const PropertyObject& source);

  void PausePolling();
  void ResumePolling();

  void UpdateFrom(const CapabilityObject& source);
  void UpdateFrom(const PropertyObject& source);

  QString name;
  QVariantMap state;
  QVariantMap parameters;

  enum UnitType {
    Capability, Property
  };

  UnitType type;

  QString cap_type;

  bool is_pending = false;
  double polling_start_time = 0.f;
  double polling_finish_time = 0.f;

  void SetPollingStartTime();
  void SetPollingEndTime();

  void SetPending(bool value);
  void SetPollingStartTime(double time);
  void SetPollingEndTime(double time);
  [[nodiscard]] bool IsInsideInterval(double time, double delta) const;
};
