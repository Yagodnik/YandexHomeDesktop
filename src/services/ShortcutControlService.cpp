#include "ShortcutControlService.h"
#include "serialization/Serialization.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QPointer>
#include <QUuid>

ShortcutControlService::ShortcutControlService(Settings* settings,
                                               WorkerLaunchConfiguration configuration,
                                               QObject* parent)
    : QObject(parent), settings_(settings), worker_(std::move(configuration), this),
      client_(QUuid::createUuid().toString(QUuid::Id128)) {
  connect(&worker_, &WorkerControlClient::finished, this, &ShortcutControlService::Complete);
  heartbeat_.setInterval(2'000);
  connect(&heartbeat_, &QTimer::timeout, this, &ShortcutControlService::Refresh);
  connect(settings_, &Settings::keybindsChanged, this, [this](const QString& account) {
    if (account.isEmpty() || account == account_id_) {
      retry_blocked_ = false;
      Refresh();
    }
  });
  recording_deadline_.setSingleShot(true);
  connect(&recording_deadline_, &QTimer::timeout, this, [this] {
    recording_ready_ = false;
    emit changed();
  });
  heartbeat_.start();
  QTimer::singleShot(0, this, &ShortcutControlService::Refresh);
}

bool ShortcutControlService::IsEnabled() const {
  return settings_->GetShortcutsEnabled();
}
bool ShortcutControlService::IsBackground() const {
  return settings_->GetShortcutsBackground();
}
bool ShortcutControlService::IsBusy() const {
  return phase_ != Phase::Idle;
}
bool ShortcutControlService::IsRecordingReady() const {
  return (!IsEnabled() && reconciled_ && phase_ == Phase::Idle && !stop_pending_) ||
         recording_ready_;
}
QJsonObject ShortcutControlService::GetStatus() const {
  return status_;
}

void ShortcutControlService::SetAccountId(const QString& account) {
  if (account_id_ == account)
    return;
  const bool replacing = !account_id_.isEmpty();
  account_id_ = account;
  retry_blocked_ = false;
  recording_ready_ = false;
  if (replacing)
    Stop();
  else
    Refresh();
}

void ShortcutControlService::SetEnabled(bool enabled) {
  settings_->SetShortcutsEnabled(enabled);
  retry_blocked_ = false;
  failure_ = {};
  recording_ready_ = false;
  if (!enabled)
    Stop();
  else
    Refresh();
}

void ShortcutControlService::SetBackground(bool enabled) {
  settings_->SetShortcutsBackground(enabled);
  Refresh();
}

void ShortcutControlService::BeginRecording() {
  ++recordings_;
  recording_ready_ = false;
  Refresh();
}

void ShortcutControlService::EndRecording() {
  if (recordings_ > 0)
    --recordings_;
  recording_ready_ = false;
  Refresh();
}

void ShortcutControlService::Refresh() {
  if (shutdown_)
    return;
  dirty_ = true;
  const QPointer<ShortcutControlService> guard(this);
  emit changed();
  if (guard)
    Pump();
}

void ShortcutControlService::Pump() {
  if (phase_ != Phase::Idle || !dirty_ || shutdown_)
    return;
  dirty_ = false;
  if (stop_pending_) {
    Stop();
    return;
  }
  if (!IsEnabled() && !reconciled_) {
    // Recover a disabled preference even if the previous GUI died before its
    // stop request reached a background worker.
    reconciled_ = true;
    Stop();
    return;
  }
  if (retry_blocked_ || !IsEnabled() || account_id_.isEmpty()) {
    emit changed();
    return;
  }
  phase_ = Phase::Starting;
  const QPointer<ShortcutControlService> guard(this);
  emit changed();
  if (guard)
    worker_.Request(WorkerControlClient::Command::Start);
}

void ShortcutControlService::Stop() {
  stop_pending_ = true;
  reconciled_ = true;
  recording_ready_ = false;
  recording_deadline_.stop();
  phase_ = Phase::Stopping;
  const QPointer<ShortcutControlService> guard(this);
  emit changed();
  if (guard)
    worker_.Request(WorkerControlClient::Command::Stop);
}

void ShortcutControlService::Shutdown() {
  shutdown_ = true;
  heartbeat_.stop();
  if (phase_ == Phase::Idle)
    Release();
}

void ShortcutControlService::Release() {
  phase_ = Phase::Releasing;
  worker_.Request(WorkerControlClient::Command::Send, {{"command", "release"}, {"client", client_}},
                  {}, 3'000);
}

void ShortcutControlService::Complete(QJsonObject result) {
  const auto phase = std::exchange(phase_, Phase::Idle);
  status_ = result;
  if (phase == Phase::Stopping) {
    stop_pending_ = !result["ok"].toBool();
    if (!failure_.isEmpty() && !stop_pending_)
      status_ = failure_;
  }
  if (phase == Phase::Releasing) {
    if (result["ok"].toBool() && !result["running"].toBool()) {
      phase_ = Phase::Stopping;
      worker_.Request(WorkerControlClient::Command::Stop, {}, {}, 3'000);
      return;
    }
    emit shutdownFinished();
    return;
  }
  if (shutdown_) {
    if (phase == Phase::Stopping)
      emit shutdownFinished();
    else
      Release();
    return;
  }
  if (phase == Phase::Starting && result["ok"].toBool()) {
    const auto bindings = settings_->GetKeybinds(account_id_);
    if (!bindings) {
      retry_blocked_ = true;
      failure_ = {{"ok", false}, {"error", QJsonObject{{"code", "invalid_bindings"}}}};
      // Invalid persisted data must release existing registrations too.
      Stop();
      return;
    }
    QJsonArray encoded;
    for (const auto& binding : *bindings)
      encoded.append(Serialization::To(binding));
    const QJsonObject configuration{{"command", "configure"},       {"client", client_},
                                    {"account_id", account_id_},    {"bindings", encoded},
                                    {"background", IsBackground()}, {"recording", recordings_ > 0}};
    if (encoded.size() > 256 ||
        QJsonDocument(configuration).toJson(QJsonDocument::Compact).size() > 60 * 1024) {
      retry_blocked_ = true;
      failure_ = {{"ok", false}, {"error", QJsonObject{{"code", "too_many_bindings"}}}};
      Stop();
      return;
    }
    phase_ = Phase::Syncing;
    worker_.Request(WorkerControlClient::Command::Send, configuration);
    return;
  }
  if (phase == Phase::Syncing && !result["ok"].toBool()) {
    retry_blocked_ = true;
    failure_ = result;
    Stop();
    return;
  }
  if (phase == Phase::Starting && !result["ok"].toBool()) {
    const auto code = result["error"].toObject()["code"].toString();
    retry_blocked_ = code.startsWith("authorization_") || code == "account_error";
  }
  if (phase == Phase::Syncing)
    failure_ = {};
  recording_ready_ = phase == Phase::Syncing && result["ok"].toBool() &&
                     result["paused"].toBool() && recordings_ > 0 && !dirty_;
  if (recording_ready_)
    recording_deadline_.start(5'000);
  // Failed startups are retried by the heartbeat, never by a tight event-loop spin.
  const QPointer<ShortcutControlService> guard(this);
  emit changed();
  if (guard && result["ok"].toBool())
    Pump();
}
