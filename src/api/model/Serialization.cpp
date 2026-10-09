#include "Actions.h"
#include "Response.h"
#include "UserInfo.h"
#include "serialization/Serialization.h"

template YANDEXAPIADAPTER_EXPORT CapabilityObject Serialization::From<CapabilityObject>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT QJsonObject Serialization::To<CapabilityObject>(const CapabilityObject&);
template YANDEXAPIADAPTER_EXPORT PropertyObject Serialization::From<PropertyObject>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT RoomObject Serialization::From<RoomObject>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT GroupObject Serialization::From<GroupObject>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT DeviceObject Serialization::From<DeviceObject>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT DeviceInfo Serialization::From<DeviceInfo>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT ScenarioObject Serialization::From<ScenarioObject>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT HouseholdObject Serialization::From<HouseholdObject>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT UserInfo Serialization::From<UserInfo>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT Response Serialization::From<Response>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT ActionResult Serialization::From<ActionResult>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT CapabilityResponseState Serialization::From<CapabilityResponseState>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT CapabilityResponse Serialization::From<CapabilityResponse>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT DeviceActionsResponse Serialization::From<DeviceActionsResponse>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT DeviceActionResponse Serialization::From<DeviceActionResponse>(const QJsonObject&);
template YANDEXAPIADAPTER_EXPORT QJsonObject Serialization::To<DeviceActionsObject>(const DeviceActionsObject&);
