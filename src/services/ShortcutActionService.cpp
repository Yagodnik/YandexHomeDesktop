#include "ShortcutActionService.h"
#include "utils/Settings.h"
#include "iot/core/CapabilityRules.h"
#include "iot/core/CapabilityState.h"
#include <QTimer>
#include <algorithm>
#include <cmath>

namespace {
std::optional<QVariantMap> State(const KeybindTarget& target, const CapabilityObject& capability) {
  const auto type = capability.type;
  if (type == CapabilityType::OnOff || type == CapabilityType::Toggle) {
    const auto value = capability.state.value("value");
    if (!capability.retrievable || value.typeId() != QMetaType::Bool ||
        capability.state.value("instance").toString() != target.instance)
      return {};
    return Iot::State::Boolean(target.instance, !value.toBool());
  }
  if (type == CapabilityType::Range) {
    const Iot::RangeParameters parameters(capability.parameters);
    auto step = parameters.Limits().Precision();
    if (!(step > 0) || !std::isfinite(step))
      step = 1;
    if (target.action == "decrease")
      step = -step;
    // Relative commands avoid reading a stale value and support relative-only devices.
    return Iot::State::RelativeRange(target.instance, step);
  }
  if (type == CapabilityType::Mode)
    return Iot::State::Mode(target.instance, target.value);
  if (target.action == "set_scene")
    return Iot::State::Scene(target.value);
  if (target.action == "set_temperature")
    return Iot::State::Temperature(target.value.toInt());
  if (target.action == "set_color") {
    const auto rgb = target.value.mid(1).toUInt(nullptr, 16);
    if (Iot::ColorParameters(capability.parameters).Model() != "hsv")
      return Iot::State::Rgb(rgb);
    const double r = ((rgb >> 16) & 255) / 255., g = ((rgb >> 8) & 255) / 255.,
                 b = (rgb & 255) / 255.;
    const auto maximum = std::max({r, g, b}), minimum = std::min({r, g, b}),
               delta = maximum - minimum;
    double hue = 0;
    if (delta > 0) {
      hue = maximum == r   ? std::fmod((g - b) / delta, 6.)
            : maximum == g ? (b - r) / delta + 2
                           : (r - g) / delta + 4;
      hue *= 60;
      if (hue < 0)
        hue += 360;
    }
    return Iot::State::Hsv(
        {{"h", hue}, {"s", maximum == 0 ? 0 : delta / maximum * 100}, {"v", maximum * 100}});
  }
  return {};
}
} // namespace

struct ShortcutActionService::Operation : QObject {
  explicit Operation(QObject* parent) : QObject(parent), request_context(new QObject(this)) {}
  QObject* request_context;
};

ShortcutActionService::ShortcutActionService(DeviceService* devices, QObject* parent,
                                             int timeout_ms)
    : ShortcutActionService(devices, nullptr, parent, timeout_ms) {}

ShortcutActionService::ShortcutActionService(DeviceService* devices, ScenarioService* scenarios,
                                             QObject* parent, int timeout_ms)
    : QObject(parent), devices_(devices), scenarios_(scenarios), timeout_ms_(timeout_ms) {}

void ShortcutActionService::Reset() {
  const auto operations = std::exchange(pending_, {});
  for (auto operation : operations)
    delete operation;
}

void ShortcutActionService::Finish(const OperationKey& key, Operation* operation,
                                   const QString& error) {
  if (pending_.value(key) != operation)
    return;
  pending_.remove(key);
  // Invalidate both steps of a scenario run immediately, including a late list
  // response delivered reentrantly by a completion observer at the deadline.
  delete std::exchange(operation->request_context, nullptr);
  // A fake/live API can complete inline from an operation's own event handler.
  operation->setParent(nullptr);
  operation->deleteLater();
  emit finished(key.second, error);
}

void ShortcutActionService::Activate(const KeybindTarget& target) {
  if (!Settings::IsValidKeybindTarget(target))
    return;
  const bool scenario = !target.scenario_id.isEmpty();
  const OperationKey key{scenario, scenario ? target.scenario_id : target.device_id};
  if ((scenario ? !scenarios_ : !devices_) || pending_.contains(key))
    return;
  const auto operation = new Operation(this);
  pending_.insert(key, operation);
  const QPointer<Operation> guard(operation);
  const QPointer<ShortcutActionService> self(this);
  QTimer::singleShot(timeout_ms_, operation, [self, guard, key] {
    if (self && guard)
      self->Finish(key, guard, "timeout");
  });
  if (scenario) {
    scenarios_->RunActiveScenario(
        target.scenario_id, operation->request_context, [self, guard, key](CommandResult result) {
          if (!self || !guard)
            return;
          QString error;
          if (!result) {
            const auto rejection = std::get_if<CommandRejection>(&result.error());
            error = !rejection ? "api_error"
                    : rejection->reason == CommandRejectionReason::InactiveScenario
                        ? "scenario_inactive"
                        : "scenario_unavailable";
          }
          self->Finish(key, guard, error);
        });
    return;
  }
  devices_->GetDeviceInfo(
      target.device_id, operation->request_context,
      [self, guard, target, key](ApiResult<DeviceInfo> info) {
        if (!self || !guard || self->pending_.value(key) != guard)
          return;
        const auto fail = [&](const QString& error) { self->Finish(key, guard, error); };
        if (!info) {
          fail("api_error");
          return;
        }
        if (info->id != target.device_id || info->status != Status::Ok ||
            info->state != DeviceState::Online) {
          fail("device_unavailable");
          return;
        }
        const CapabilityObject* match = nullptr;
        QVariantMap state;
        const auto rules =
            Iot::Rules::ForType(CapabilityType(CapabilityType::AsValue(target.capability_type)));
        if (!rules) {
          fail("capability_unavailable");
          return;
        }
        for (const auto& capability : info->capabilities) {
          if (capability.type != rules->Type())
            continue;
          const auto candidate = State(target, capability);
          const auto selector = candidate.value_or(Iot::State::Boolean(target.instance, false));
          if (!rules->Matches(capability, selector))
            continue;
          if (match) {
            fail("ambiguous_capability");
            return;
          }
          match = &capability;
          state = candidate.value_or(QVariantMap{});
        }
        if (!match || state.isEmpty()) {
          fail("capability_unavailable");
          return;
        }
        if (!Iot::ValidateInput(*rules, state) ||
            !rules->ValidateDevice(match->parameters, state)) {
          fail("invalid_value");
          return;
        }
        if (!self->devices_) {
          fail("device_unavailable");
          return;
        }
        self->devices_->UseCapability(
            target.device_id, rules->Type(), state, guard->request_context,
            [self, guard, key](ApiResult<void> result) {
              if (self && guard)
                self->Finish(key, guard, result ? QString{} : QString("api_error"));
            });
      });
}
