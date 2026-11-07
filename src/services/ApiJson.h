#pragma once

#include "yh/appservices_export.h"
#include <QJsonObject>
#include "api/model/UserInfo.h"

APPSERVICES_EXPORT QJsonObject CapabilityJson(const CapabilityObject& capability);
APPSERVICES_EXPORT QJsonObject DeviceJson(const DeviceInfo& device);
