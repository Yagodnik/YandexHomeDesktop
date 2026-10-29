#include "McpCommand.h"

#include <QJsonArray>
#include <optional>
#include "services/ApiJson.h"
#include "services/DeviceService.h"
#include "services/HomeService.h"
#include "services/ScenarioService.h"

namespace {
QJsonObject Error(const QString& message) { return {{"ok", false}, {"error", message}}; }

QJsonObject CommandErrorResult(const CommandError& error) {
  if (const auto* api = std::get_if<ApiError>(&error)) return Error(api->message);
  const auto& rejection = std::get<CommandRejection>(error);
  return Error(rejection.message.isEmpty() ? QStringLiteral("The command was rejected.")
                                           : rejection.message);
}

class ListDevices final : public McpCommand {
public:
  explicit ListDevices(HomeService* home) : home_(home) {}
  void Execute(const QJsonObject&, QObject* context, Completion complete) const override {
    home_->ReadHome(context, [complete = std::move(complete)](ApiResult<UserInfo> result) {
      if (!result) { complete(Error(result.error().message)); return; }
      if (result->status != Status::Ok) { complete(Error(result->message)); return; }
      QJsonArray devices, rooms, households;
      for (const auto& d : result->devices)
        devices.append(QJsonObject{{"id", d.id}, {"name", d.name}, {"type", d.type},
                                   {"room", d.room}, {"household_id", d.household_id},
                                   {"aliases", QJsonArray::fromStringList(d.aliases)}});
      for (const auto& r : result->rooms)
        rooms.append(QJsonObject{{"id", r.id}, {"name", r.name}, {"household_id", r.household_id}});
      for (const auto& h : result->households)
        households.append(QJsonObject{{"id", h.id}, {"name", h.name}});
      complete({{"ok", true}, {"devices", devices}, {"rooms", rooms}, {"households", households}});
    });
  }
private:
  HomeService* home_;
};

class ListScenarios final : public McpCommand {
public:
  explicit ListScenarios(ScenarioService* scenarios) : scenarios_(scenarios) {}
  void Execute(const QJsonObject&, QObject* context, Completion complete) const override {
    scenarios_->ListScenarios(context, [complete = std::move(complete)](ApiResult<QList<ScenarioObject>> result) {
      if (!result) { complete(Error(result.error().message)); return; }
      QJsonArray items;
      for (const auto& s : *result)
        items.append(QJsonObject{{"id", s.id}, {"name", s.name}, {"is_active", s.is_active}});
      complete({{"ok", true}, {"scenarios", items}});
    });
  }
private:
  ScenarioService* scenarios_;
};

class GetDevice final : public McpCommand {
public:
  explicit GetDevice(DeviceService* devices) : devices_(devices) {}
  void Execute(const QJsonObject& args, QObject* context, Completion complete) const override {
    const auto device = args.value("device_id").toString();
    if (device.isEmpty()) { complete(Error("device_id is required.")); return; }
    devices_->GetDeviceInfo(device, context, [device, complete = std::move(complete)](ApiResult<DeviceInfo> result) {
      if (!result) complete(Error(result.error().message));
      else if (result->status != Status::Ok || result->id != device) complete(Error("Invalid device response."));
      else complete({{"ok", true}, {"device", DeviceJson(*result)}});
    });
  }
private:
  DeviceService* devices_;
};

class RunScenario final : public McpCommand {
public:
  explicit RunScenario(ScenarioService* scenarios) : scenarios_(scenarios) {}
  void Execute(const QJsonObject& args, QObject* context, Completion complete) const override {
    const auto scenario = args.value("scenario_id").toString();
    if (scenario.isEmpty()) { complete(Error("scenario_id is required.")); return; }
    scenarios_->RunActiveScenario(scenario, context, [scenario, complete = std::move(complete)](CommandResult result) {
      complete(result ? QJsonObject{{"ok", true}, {"scenario_id", scenario}} : CommandErrorResult(result.error()));
    });
  }
private:
  ScenarioService* scenarios_;
};

class ControlDevice final : public McpCommand {
public:
  explicit ControlDevice(DeviceService* devices) : devices_(devices) {}
  void Execute(const QJsonObject& args, QObject* context, Completion complete) const override {
    const auto device = args.value("device_id").toString();
    const auto capability = args.value("capability").toString();
    const auto state = args.value("state");
    std::optional<CapabilityType> type;
    for (const auto candidate : {CapabilityType::OnOff, CapabilityType::Range, CapabilityType::Toggle,
                                 CapabilityType::Mode, CapabilityType::ColorSetting}) {
      const auto name = CapabilityType::AsString(candidate);
      if (capability == name || capability == name.sliced(QString("devices.capabilities.").size())) type = candidate;
    }
    if (device.isEmpty() || !type || !state.isObject()) {
      complete(Error("device_id, capability and state are required."));
      return;
    }
    devices_->ApplyCapability(device, *type, state.toObject().toVariantMap(), context,
      [device, complete = std::move(complete)](CommandResult result) {
        complete(result ? QJsonObject{{"ok", true}, {"device_id", device}} : CommandErrorResult(result.error()));
      });
  }
private:
  DeviceService* devices_;
};
} // namespace

McpCommands CreateMcpCommands(HomeService* home, DeviceService* devices, ScenarioService* scenarios) {
  const auto schema = [](QJsonObject properties, QJsonArray required) {
    return QJsonObject{{"type", "object"}, {"properties", properties},
                       {"required", required}, {"additionalProperties", false}};
  };
  const QJsonObject device_id{{"type", "string"}, {"description", "Device ID from list_devices."}};
  return {
      {"list_devices", {"List devices, aliases, rooms and households. Match the user's words against device names, aliases and room names; join device room IDs to rooms. If multiple devices match, ask which one before acting.",
                        schema({}, {}), std::make_shared<ListDevices>(home)}},
      {"get_device", {"Get a device's current state and advertised capabilities, instances and supported values. Use this before controlling a device.",
                      schema({{"device_id", device_id}}, {"device_id"}), std::make_shared<GetDevice>(devices)}},
      {"control_device", {"Apply one capability action to a device by ID. Choose the instance and value from get_device. For turning on use capability on_off and state {instance: on, value: true}. Success means the API accepted the action, not physical confirmation.",
                          schema({{"device_id", device_id},
                                  {"capability", QJsonObject{{"type", "string"}, {"enum", QJsonArray{
                                      "on_off", "range", "toggle", "mode", "color_setting",
                                      "devices.capabilities.on_off", "devices.capabilities.range",
                                      "devices.capabilities.toggle", "devices.capabilities.mode",
                                      "devices.capabilities.color_setting"}}}},
                                  {"state", QJsonObject{
                                      {"type", "object"},
                                      {"description", "Capability instance and value; range actions may include relative."},
                                      {"properties", QJsonObject{
                                          {"instance", QJsonObject{{"type", "string"}}},
                                          {"value", QJsonObject{{"description", "Value supported by this capability, such as a boolean, number, mode, or color object."}}},
                                          {"relative", QJsonObject{{"type", "boolean"}}}}},
                                      {"required", QJsonArray{"instance", "value"}},
                                      {"additionalProperties", false}}}},
                                 {"device_id", "capability", "state"}), std::make_shared<ControlDevice>(devices)}},
      {"list_scenarios", {"List scenarios and whether they are active.", schema({}, {}), std::make_shared<ListScenarios>(scenarios)}},
      {"run_scenario", {"Run an active scenario by ID from list_scenarios. Ask the user when its name is ambiguous.",
                        schema({{"scenario_id", QJsonObject{{"type", "string"}}}}, {"scenario_id"}), std::make_shared<RunScenario>(scenarios)}}};
}

QJsonArray DescribeMcpCommands(const McpCommands& commands) {
  QJsonArray tools;
  auto names = commands.keys();
  names.sort();
  for (const auto& name : names) {
    const auto& tool = commands[name];
    tools.append(QJsonObject{{"name", name}, {"description", tool.description},
                             {"inputSchema", tool.input_schema}});
  }
  return tools;
}
