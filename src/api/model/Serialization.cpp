#include "Actions.h"
#include "Response.h"
#include "UserInfo.h"
#include "serialization/Serialization.h"

template CapabilityObject Serialization::From<CapabilityObject>(const QJsonObject&);
template QJsonObject Serialization::To<CapabilityObject>(const CapabilityObject&);
template PropertyObject Serialization::From<PropertyObject>(const QJsonObject&);
template RoomObject Serialization::From<RoomObject>(const QJsonObject&);
template GroupObject Serialization::From<GroupObject>(const QJsonObject&);
template DeviceObject Serialization::From<DeviceObject>(const QJsonObject&);
template DeviceInfo Serialization::From<DeviceInfo>(const QJsonObject&);
template ScenarioObject Serialization::From<ScenarioObject>(const QJsonObject&);
template HouseholdObject Serialization::From<HouseholdObject>(const QJsonObject&);
template UserInfo Serialization::From<UserInfo>(const QJsonObject&);
template Response Serialization::From<Response>(const QJsonObject&);
template ActionResult Serialization::From<ActionResult>(const QJsonObject&);
template CapabilityResponseState Serialization::From<CapabilityResponseState>(const QJsonObject&);
template CapabilityResponse Serialization::From<CapabilityResponse>(const QJsonObject&);
template DeviceActionsResponse Serialization::From<DeviceActionsResponse>(const QJsonObject&);
template DeviceActionResponse Serialization::From<DeviceActionResponse>(const QJsonObject&);
template QJsonObject Serialization::To<DeviceActionsObject>(const DeviceActionsObject&);
