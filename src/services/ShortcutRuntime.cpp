#include "ShortcutRuntime.h"
#include "utils/Settings.h"
#include "serialization/Serialization.h"
#include <QJsonArray>
#include <QSet>
#include <QPointer>
#include <cmath>
#include <climits>

namespace {
QJsonObject Error(const QString& code) {
  return {{"ok", false}, {"error", QJsonObject{{"code", code}}}};
}
std::optional<QList<KeybindAssignment>> Decode(const QJsonValue& value) {
  if (!value.isArray() || value.toArray().size() > 256)
    return {};
  QList<KeybindAssignment> bindings;
  QSet<int> combinations;
  for (const auto& entry : value.toArray()) {
    if (!entry.isObject())
      return {};
    auto object = entry.toObject();
    if (!object["combination"].isDouble() || !object["target"].isObject() ||
        !object["device_name"].isString() || !object["action_label"].isString())
      return {};
    auto target = object["target"].toObject();
    if (!target.contains("scenario_id"))
      target.insert("scenario_id", "");
    object["target"] = target;
    for (const auto* field :
         {"device_id", "capability_type", "instance", "action", "value", "scenario_id"})
      if (!target[field].isString())
        return {};
    const auto numeric = object["combination"].toDouble();
    if (numeric < 1 || numeric > INT_MAX || numeric != std::floor(numeric))
      return {};
    const auto binding = Serialization::From<KeybindAssignment>(object);
    const auto key = binding.combination & ~Qt::KeyboardModifierMask;
    const auto modifiers = binding.combination & Qt::KeyboardModifierMask;
    if (!Settings::IsValidKeybindTarget(binding.target) ||
        Settings::NormalizeKeyCombination(key, modifiers) != binding.combination ||
        combinations.contains(binding.combination))
      return {};
    for (const auto& existing : bindings)
      if (Settings::SameKeybindTarget(existing.target, binding.target))
        return {};
    combinations.insert(binding.combination);
    bindings.append(binding);
  }
  return bindings;
}
} // namespace

ShortcutRuntime::ShortcutRuntime(IShortcutRegistrar* registrar, ShortcutActionService* actions,
                                 QString account_id, QObject* parent, int lease_ms)
    : QObject(parent), registrar_(registrar), actions_(actions), account_id_(std::move(account_id)),
      lease_ms_(lease_ms) {
  clock_.start();
  // The initial launch is also leased, including a controller that dies before sync.
  leases_.insert({}, {lease_ms_, false});
  timer_.setInterval(std::max(1, lease_ms_ / 4));
  connect(&timer_, &QTimer::timeout, this, &ShortcutRuntime::Expire);
  connect(actions_, &ShortcutActionService::finished, this,
          [this](const QString&, const QString& error) {
            action_error_ = error;
            if (error.isEmpty())
              ++completed_;
            else
              ++failed_;
          });
  timer_.start();
}

QJsonObject ShortcutRuntime::Status() const {
  return {{"ok", true},
          {"running", running_},
          {"paused", paused_ && released_},
          {"release_failed", !released_},
          {"registered", registered_},
          {"conflicts", conflicts_},
          {"conflict_count", conflict_count_},
          {"action_error", action_error_},
          {"completed", completed_},
          {"failed", failed_}};
}

ShortcutRuntime::~ShortcutRuntime() {
  actions_->Reset();
  registrar_->Clear();
}

void ShortcutRuntime::Stop() {
  if (!running_)
    return;
  running_ = false;
  ++generation_;
  timer_.stop();
  actions_->Reset();
  released_ = registrar_->Clear();
  registered_ = 0;
  emit stopRequested();
}

void ShortcutRuntime::Expire() {
  for (auto it = leases_.begin(); it != leases_.end();) {
    if (it->expires <= clock_.elapsed())
      it = leases_.erase(it);
    else
      ++it;
  }
  if (leases_.isEmpty() && !background_) {
    Stop();
    return;
  }
  Apply();
}

void ShortcutRuntime::Apply() {
  bool pause = false;
  for (const auto& lease : leases_)
    pause |= lease.recording;
  if (applied_ && pause == paused_)
    return;
  applied_ = true;
  paused_ = pause;
  const auto generation = ++generation_;
  const QPointer<ShortcutRuntime> self(this);
  actions_->Reset();
  const auto released = registrar_->Clear();
  if (!self || self->generation_ != generation)
    return;
  released_ = released;
  registered_ = 0;
  conflicts_ = {};
  conflict_count_ = 0;
  if (!released_) {
    applied_ = false;
    return;
  }
  if (paused_ || !running_)
    return;
  const auto bindings = bindings_;
  for (const auto& binding : bindings) {
    const bool registered =
        registrar_->Register(binding.combination, [self, generation, target = binding.target] {
          if (self && self->generation_ == generation && self->running_ && !self->paused_)
            self->actions_->Activate(target);
        });
    if (!self || self->generation_ != generation)
      return;
    if (registered)
      ++registered_;
    else {
      ++conflict_count_;
      if (conflicts_.size() < 8)
        conflicts_.append(QJsonObject{{"combination", binding.combination},
                                      {"device_name", binding.device_name.left(128)},
                                      {"action_label", binding.action_label.left(128)}});
    }
  }
}

QJsonObject ShortcutRuntime::Handle(const QJsonObject& request) {
  const auto command = request["command"].toString();
  const QPointer<ShortcutRuntime> guard(this);
  const QJsonObject stopped{{"ok", true}, {"running", false}};
  if (command == "stop") {
    Stop();
    return guard ? Status() : stopped;
  }
  if (command == "status")
    return Status();
  if (!running_)
    return Error("stopping");
  const auto client = request["client"].toString();
  if (client.isEmpty() || client.size() > 128)
    return Error("invalid_client");
  if (command == "release") {
    leases_.remove(client);
    leases_.remove({});
    Expire();
    return guard ? Status() : stopped;
  }
  if (command != "configure")
    return Error("unknown_command");
  if (request["account_id"].toString() != account_id_ || account_id_.isEmpty())
    return Error("account_mismatch");
  if (!request["background"].isBool() || !request["recording"].isBool())
    return Error("invalid_request");
  const auto bindings = Decode(request["bindings"]);
  if (!bindings)
    return Error("invalid_bindings");
  const auto encoded = request["bindings"].toArray();
  if (encoded != encoded_bindings_) {
    bindings_ = *bindings;
    encoded_bindings_ = encoded;
    applied_ = false;
    action_error_.clear();
  }
  leases_.remove({});
  leases_.insert(client, {clock_.elapsed() + lease_ms_, request["recording"].toBool()});
  background_ = request["background"].toBool();
  Apply();
  return guard ? Status() : stopped;
}
