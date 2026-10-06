#include "DeviceCommands.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <algorithm>
#include "cli/CliContext.h"
#include "cli/CliOutput.h"

namespace {
using CommandResult = CommandRegistry::Result;

bool CheckHousehold(CliContext& context, const UserInfo& home, const QString& id) {
  if (!id.isEmpty() && std::none_of(home.households.begin(), home.households.end(),
      [&](const auto& household) { return household.id == id; })) {
    context.Fail(CliContext::NotFound, "household_not_found", DeviceCommands::tr("Дом с таким ID не найден."));
    return false;
  }
  return true;
}
QJsonObject DeviceSummary(const DeviceObject& device) {
  return {{"id", device.id}, {"name", device.name}, {"type", device.type},
    {"room", device.room}, {"household_id", device.household_id}};
}

class ListDevicesCommand final : public ICommand {
public:
  explicit ListDevicesCommand(QString household) : household_(std::move(household)) {}
  static CommandResult Parse(const CliArguments& arguments) {
    const auto household = arguments.Value("household");
    if (arguments.Has("household") && household.isEmpty()) {
      return std::unexpected(CliArguments::tr("ID дома не должен быть пустым."));
    }
    return std::make_shared<ListDevicesCommand>(household);
  }
  void Execute(CliContext& context) const override {
    context.Services().home->ReadHome(context.Owner(), [this, &context](ApiResult<UserInfo> result) {
      if (!result) { context.FailApi(result.error()); return; }
      if (result->status != Status::Ok) { context.FailApi({ApiErrorKind::Service, result->message}); return; }
      if (!CheckHousehold(context, *result, household_)) { return; }
      QJsonArray devices;
      QString text = DeviceCommands::tr("ID\tИмя\tТип\tДом\n");
      for (const auto& device : result->devices) {
        if (!household_.isEmpty() && device.household_id != household_) { continue; }
        devices.append(DeviceSummary(device));
        text += QString("%1\t%2\t%3\t%4\n").arg(device.id, device.name, device.type, device.household_id);
      }
      context.Complete({{"devices", devices}}, text);
    });
  }
private:
  QString household_;
};

class DeviceTargetCommand : public ICommand {
public:
  explicit DeviceTargetCommand(CliTarget target) : target_(std::move(target)) {}
  void Execute(CliContext& context) const final {
    if (!target_.id.isEmpty()) { ReadDevice(context, target_.id); return; }
    context.Services().home->ReadHome(context.Owner(), [this, &context](ApiResult<UserInfo> result) {
      if (!result) { context.FailApi(result.error()); return; }
      if (result->status != Status::Ok) { context.FailApi({ApiErrorKind::Service, result->message}); return; }
      if (!CheckHousehold(context, *result, target_.household)) { return; }
      QJsonArray matches;
      for (const auto& device : result->devices) {
        if (device.name == target_.name && (target_.household.isEmpty() || device.household_id == target_.household)) {
          matches.append(DeviceSummary(device));
        }
      }
      if (matches.isEmpty()) { context.Fail(CliContext::NotFound, "device_not_found", DeviceCommands::tr("Устройство с таким именем не найдено.")); }
      else if (matches.size() > 1) {
        context.Fail(CliContext::Usage, "ambiguous_target", DeviceCommands::tr("Найдено несколько устройств. Укажите --id или --household."), {{"matches", matches}});
      } else { ReadDevice(context, matches[0].toObject()["id"].toString()); }
    });
  }
protected:
  virtual void OnDevice(CliContext& context, const DeviceInfo& info) const = 0;
private:
  void ReadDevice(CliContext& context, const QString& id) const {
    context.Services().devices->GetDeviceInfo(id, context.Owner(), [this, &context, id](ApiResult<DeviceInfo> result) {
      if (!result) { context.FailApi(result.error()); return; }
      if (result->id != id) { context.FailApi({ApiErrorKind::InvalidResponse, DeviceCommands::tr("Ответ содержит другое устройство.")}); return; }
      if (result->status != Status::Ok) { context.FailApi({ApiErrorKind::Service, result->message}); return; }
      OnDevice(context, *result);
    });
  }
  CliTarget target_;
};

class ShowDeviceCommand final : public DeviceTargetCommand {
public:
  using DeviceTargetCommand::DeviceTargetCommand;
  static CommandResult Parse(const CliArguments& arguments) {
    const auto target = CliTarget::Parse(arguments, true);
    if (!target) { return std::unexpected(target.error()); }
    return std::make_shared<ShowDeviceCommand>(*target);
  }
private:
  void OnDevice(CliContext& context, const DeviceInfo& info) const override {
    const auto device = DeviceJson(info);
    context.Complete({{"device", device}}, QString::fromUtf8(QJsonDocument(device).toJson(QJsonDocument::Indented)));
  }
};

class SetDeviceCommand final : public DeviceTargetCommand {
public:
  SetDeviceCommand(CliTarget target, CapabilityAction action)
    : DeviceTargetCommand(std::move(target)), action_(std::move(action)) {}
  static CommandResult Parse(const CliArguments& arguments, const CapabilityCommands& inputs) {
    const auto target = CliTarget::Parse(arguments, true);
    if (!target) { return std::unexpected(target.error()); }
    const auto action = inputs.Parse(arguments);
    if (!action) { return std::unexpected(action.error()); }
    return std::make_shared<SetDeviceCommand>(*target, *action);
  }
private:
  void OnDevice(CliContext& context, const DeviceInfo& info) const override {
    QList<CapabilityObject> matches;
    for (const auto& capability : info.capabilities) {
      if (action_.rules->Matches(capability, action_.state)) { matches.append(capability); }
    }
    if (matches.isEmpty()) {
      context.Fail(CliContext::NotFound, "capability_not_found", DeviceCommands::tr("Устройство не поддерживает это умение и экземпляр."));
      return;
    }
    if (matches.size() > 1) {
      context.Fail(CliContext::Usage, "ambiguous_capability", DeviceCommands::tr("Устройство содержит несколько одинаковых умений."));
      return;
    }
    const auto valid = action_.rules->ValidateDevice(matches[0].parameters, action_.state);
    if (!valid) { context.Fail(CliContext::Usage, "invalid_value", valid.error()); return; }
    context.Services().devices->UseCapability(info.id, action_.rules->Type(), action_.state, context.Owner(),
      [this, &context, id = info.id](ApiResult<void> result) {
        if (!result) { context.FailApi(result.error()); return; }
        context.Complete({{"device_id", id}, {"capability", CapabilityType::AsString(action_.rules->Type())},
          {"state", QJsonObject::fromVariantMap(action_.state)}}, DeviceCommands::tr("Команда для устройства %1 выполнена.\n").arg(id));
      });
  }
  CapabilityAction action_;
};
}

void RegisterDeviceCommands(CommandRegistry& registry, const CapabilityCommands& capabilities) {
  AddTargetOptions(registry);
  registry.AddOption({"household", DeviceCommands::tr("ID дома для списка или поиска устройства по имени."), "id"});
  registry.AddOption({"capability", DeviceCommands::tr("on_off, range, mode, toggle или color_setting."), "type"});
  registry.AddOption({"instance", DeviceCommands::tr("Экземпляр умения: brightness, mute, rgb и т. п."), "instance"});
  registry.AddOption({"value", DeviceCommands::tr("Значение: on/off, число, режим, цвет или JSON HSV."), "value"});
  registry.AddOption({"relative", DeviceCommands::tr("Относительное изменение range вместо абсолютного значения.")});
  registry.AddOption({"list-devices", DeviceCommands::tr("Совместимость: devices list.")});
  registry.AddOption({"on_off", DeviceCommands::tr("Совместимость: on/off по имени устройства."), "name"});
  registry.AddOption({"info", DeviceCommands::tr("Справка для старого --on_off.")});
  registry.Register({{"devices", "list"}, DeviceCommands::tr("Список устройств с ID."),
    {"household"}, ListDevicesCommand::Parse});
  registry.Register({{"devices", "show"}, DeviceCommands::tr("Состояние, умения и свойства устройства."),
    {"id", "name", "household"}, ShowDeviceCommand::Parse});
  registry.Register({{"devices", "set"}, DeviceCommands::tr("Отправить действие устройству."),
    {"id", "name", "household", "capability", "instance", "value", "relative"},
    [capabilities](const CliArguments& arguments) { return SetDeviceCommand::Parse(arguments, capabilities); }});
  registry.RegisterLegacy({"list-devices", {"devices", "list"}, {}, {}, {}, {}});
  registry.RegisterLegacy({"on_off", {"devices", "set"}, "name", {{"capability", "on_off"}}, {"id", "name", "capability"}, "info"});
}
