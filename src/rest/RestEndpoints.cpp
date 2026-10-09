#include "RestEndpoints.h"

#include <QCoreApplication>
#include <QJsonArray>

#include "RestAction.h"
#include "RestErrors.h"
#include "RestProtocol.h"
#include "services/AccountService.h"
#include "services/ApiJson.h"
#include "services/DeviceService.h"
#include "services/HomeService.h"
#include "services/ScenarioService.h"

namespace {
void ListDevices(HomeService& home, RestReply* reply) {
  home.ReadHome(reply, [reply](ApiResult<UserInfo> result) {
    if (!result) {
      FailApi(reply, result.error());
      return;
    }
    if (result->status != Status::Ok) {
      FailApi(reply, {ApiErrorKind::Service, result->message});
      return;
    }

    QJsonArray devices;
    QJsonArray rooms;
    QJsonArray households;
    for (const auto& device : result->devices) {
      devices.append(QJsonObject{{"id", device.id}, {"name", device.name}, {"type", device.type},
        {"room", device.room}, {"household_id", device.household_id}});
    }
    for (const auto& room : result->rooms) {
      rooms.append(
        QJsonObject{{"id", room.id}, {"name", room.name}, {"household_id", room.household_id}});
    }
    for (const auto& household : result->households) {
      households.append(QJsonObject{{"id", household.id}, {"name", household.name}});
    }
    reply->Send({{"devices", devices}, {"rooms", rooms}, {"households", households}});
  });
}

void ReadDevice(DeviceService& devices, const QString& id, RestReply* reply) {
  devices.GetDeviceInfo(id, reply, [reply, id](ApiResult<DeviceInfo> result) {
    if (!result) {
      FailApi(reply, result.error());
      return;
    }
    if (result->status != Status::Ok || result->id != id) {
      FailApi(reply, {ApiErrorKind::InvalidResponse, QCoreApplication::translate("RestServer",
                                                       "Некорректный ответ устройства.")});
      return;
    }
    reply->Send({{"device", DeviceJson(*result)}});
  });
}

void ApplyAction(
  DeviceService& devices, const QString& id, const QHttpServerRequest& request, RestReply* reply) {
  const auto action = ParseRestAction(request);
  if (!action) {
    const auto& error = action.error();
    reply->Fail(error.status, error.code, error.message);
    return;
  }
  devices.ApplyCapability(
    id, action->type, action->state, reply, [reply, id, action = *action](CommandResult result) {
      if (!result) {
        FailCommand(reply, result.error());
        return;
      }
      reply->Send({{"device_id", id}, {"capability", CapabilityType::AsString(action.type)},
        {"state", QJsonObject::fromVariantMap(action.state)}});
    });
}

void ListScenarios(ScenarioService& scenarios, RestReply* reply) {
  scenarios.ListScenarios(reply, [reply](ApiResult<QList<ScenarioObject>> result) {
    if (!result) {
      FailApi(reply, result.error());
      return;
    }
    QJsonArray items;
    for (const auto& scenario : *result) {
      items.append(QJsonObject{
        {"id", scenario.id}, {"name", scenario.name}, {"is_active", scenario.is_active}});
    }
    reply->Send({{"scenarios", items}});
  });
}

void RunScenario(ScenarioService& scenarios, const QString& id, RestReply* reply) {
  scenarios.RunActiveScenario(id, reply, [reply, id](CommandResult result) {
    if (!result) {
      FailCommand(reply, result.error());
      return;
    }
    reply->Send({{"scenario_id", id}});
  });
}

void ReadAccount(AccountService& account, RestReply* reply) {
  account.ReadAccount(reply, [reply](ApiResult<AccountInfo> result) {
    if (!result) {
      FailApi(reply, result.error());
      return;
    }
    reply->Send({{"account", QJsonObject{{"id", result->id}, {"name", result->display_name},
                               {"email", result->default_email}}}});
  });
}
} // namespace

RestRouter CreateRestRoutes(
  HomeService& home, DeviceService& devices, ScenarioService& scenarios, AccountService& account) {
  using Method = QHttpServerRequest::Method;
  RestRouter routes;
  routes.Add(Method::Get, "/v1/status", [](const auto&, const auto&, RestReply* reply) {
    reply->Send({{"running", true}, {"version", RestProtocol::Version}});
  });
  routes.Add(Method::Get, "/v1/devices", [&home](const auto&, const auto&, RestReply* reply) {
    ListDevices(home, reply);
  });
  routes.Add(Method::Get, "/v1/devices/{device_id}",
    [&devices](const auto&, const auto& parameters, RestReply* reply) {
      ReadDevice(devices, parameters.value("device_id"), reply);
    });
  routes.Add(Method::Post, "/v1/devices/{device_id}/actions",
    [&devices](const auto& request, const auto& parameters, RestReply* reply) {
      ApplyAction(devices, parameters.value("device_id"), request, reply);
    });
  routes.Add(
    Method::Get, "/v1/scenarios", [&scenarios](const auto&, const auto&, RestReply* reply) {
      ListScenarios(scenarios, reply);
    });
  routes.Add(Method::Post, "/v1/scenarios/{scenario_id}/run",
    [&scenarios](const auto&, const auto& parameters, RestReply* reply) {
      RunScenario(scenarios, parameters.value("scenario_id"), reply);
    });
  routes.Add(Method::Get, "/v1/account", [&account](const auto&, const auto&, RestReply* reply) {
    ReadAccount(account, reply);
  });
  return routes;
}
