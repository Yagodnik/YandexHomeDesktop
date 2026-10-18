#include "KeybindSettingsModel.h"
#include "AccountModel.h"
#include "serialization/Serialization.h"
#include <QKeySequence>

KeybindSettingsModel::KeybindSettingsModel(Settings* settings, AccountModel* account,
                                           QObject* parent)
    : QObject(parent), settings_(settings) {
  connect(settings_, &Settings::keybindsChanged, this, [this](const QString& accountId) {
    if (accountId.isEmpty() || accountId == account_id_)
      Reload();
  });
  if (account) {
    connect(account, &AccountModel::dataLoaded, this,
            [this, account] { SetAccountId(account->GetAccountId()); });
    SetAccountId(account->GetAccountId());
  }
}

bool KeybindSettingsModel::IsAvailable() const {
  return !account_id_.isEmpty() && bindings_.has_value();
}
QString KeybindSettingsModel::GetAccountId() const {
  return account_id_;
}
int KeybindSettingsModel::GetRevision() const {
  return revision_;
}
QString KeybindSettingsModel::GetLastError() const {
  return last_error_;
}

void KeybindSettingsModel::SetAccountId(const QString& accountId) {
  if (account_id_ == accountId)
    return;
  account_id_ = accountId;
  SetError({});
  Reload();
}

void KeybindSettingsModel::Reload() {
  bindings_ = account_id_.isEmpty() ? std::nullopt : settings_->GetKeybinds(account_id_);
  ++revision_;
  emit changed();
}

void KeybindSettingsModel::SetError(const QString& error) {
  if (last_error_ == error)
    return;
  last_error_ = error;
  emit lastErrorChanged();
}

KeybindTarget KeybindSettingsModel::Target(const QVariantMap& value) {
  return Serialization::From<KeybindTarget>(QJsonObject::fromVariantMap(value));
}

int KeybindSettingsModel::GetCombination(const QVariantMap& target) const {
  if (!IsAvailable())
    return 0;
  const auto identity = Target(target);
  for (const auto& binding : *bindings_) {
    if (Settings::SameKeybindTarget(binding.target, identity))
      return binding.combination;
  }
  return 0;
}

int KeybindSettingsModel::MakeCombination(int key, int modifiers) const {
  return Settings::NormalizeKeyCombination(key, modifiers);
}

QString KeybindSettingsModel::DisplayCombination(int combination) const {
  return combination == 0 ? QString{}
                          : QKeySequence(QKeyCombination::fromCombined(combination))
                                .toString(QKeySequence::NativeText);
}

QString KeybindSettingsModel::ConflictMessage(const QVariantMap& target, int combination) const {
  if (account_id_.isEmpty())
    return tr("Дождитесь загрузки аккаунта, чтобы сохранить сочетание.");
  if (!bindings_)
    return tr("Не удалось прочитать сохранённые сочетания. Настройки не изменены.");
  const auto identity = Target(target);
  if (!Settings::IsValidKeybindTarget(identity))
    return tr("Не удалось определить действие для этого сочетания.");
  if (combination == 0)
    return {};
  for (const auto& binding : *bindings_) {
    if (binding.combination == combination &&
        !Settings::SameKeybindTarget(binding.target, identity)) {
      const auto name =
          binding.device_name.isEmpty() ? binding.target.device_id : binding.device_name;
      return binding.action_label.isEmpty()
                 ? tr("%1 уже назначено устройству «%2». Выберите другое сочетание.")
                       .arg(DisplayCombination(combination), name)
                 : tr("%1 уже назначено: %2 — %3. Выберите другое сочетание.")
                       .arg(DisplayCombination(combination), name, binding.action_label);
    }
  }
  return {};
}

bool KeybindSettingsModel::Save(const QVariantMap& target, int combination,
                                const QString& deviceName, const QString& actionLabel) {
  // Read again when saving: duplicate prevention belongs to Settings as well as the editor.
  Reload();
  const auto error = ConflictMessage(target, combination);
  if (!error.isEmpty()) {
    SetError(error);
    return false;
  }
  const auto result =
      settings_->SaveKeybind(account_id_, Target(target), combination, deviceName, actionLabel);
  switch (result) {
  case Settings::KeybindSaveResult::Saved:
    SetError({});
    return true;
  case Settings::KeybindSaveResult::Conflict:
    Reload();
    SetError(ConflictMessage(target, combination));
    return false;
  case Settings::KeybindSaveResult::Invalid:
    SetError(tr("Не удалось сохранить это сочетание."));
    return false;
  case Settings::KeybindSaveResult::StorageError:
    SetError(tr("Не удалось сохранить настройки. Попробуйте снова."));
    return false;
  }
  return false;
}
