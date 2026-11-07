#include "ShortcutViewModel.h"
#include <QJsonArray>

ShortcutViewModel::ShortcutViewModel(ShortcutControlService* service, QObject* parent)
    : QObject(parent), service_(service) {
  connect(service_, &ShortcutControlService::changed, this, &ShortcutViewModel::changed);
}
bool ShortcutViewModel::IsEnabled() const {
  return service_->IsEnabled();
}
bool ShortcutViewModel::IsBackground() const {
  return service_->IsBackground();
}
bool ShortcutViewModel::IsBusy() const {
  return service_->IsBusy();
}
void ShortcutViewModel::SetEnabled(bool enabled) {
  service_->SetEnabled(enabled);
}
void ShortcutViewModel::SetBackground(bool enabled) {
  service_->SetBackground(enabled);
}
QString ShortcutViewModel::GetStatus() const {
  const auto status = service_->GetStatus();
  if (service_->IsBusy() && !status["running"].toBool())
    return tr("Подождите…");
  if (!status["running"].toBool())
    return tr("Служба сочетаний остановлена");
  if (status["paused"].toBool())
    return tr("Приостановлено на время назначения сочетания");
  return tr("Активных сочетаний: %1").arg(status["registered"].toInt());
}
QString ShortcutViewModel::GetError() const {
  const auto status = service_->GetStatus();
  const auto code = status["error"].toObject()["code"].toString();
  if (code == "account_mismatch")
    return tr("Аккаунт службы изменился. Отключите и включите сочетания.");
  if (code == "authorization_required" || code == "authorization_failed" ||
      code == "authorization_canceled")
    return tr("Не удалось прочитать данные входа. Войдите в аккаунт через приложение.");
  if (!code.isEmpty())
    return tr("Ошибка службы сочетаний: %1").arg(code);
  if (status["release_failed"].toBool())
    return tr("Не удалось освободить сочетания для записи. Отключите службу и повторите попытку.");
  const auto conflicts = status["conflicts"].toArray();
  if (!conflicts.isEmpty()) {
    const auto first = conflicts.first().toObject();
    return tr("Не удалось зарегистрировать сочетаний: %1. Проверьте «%2 — %3» и системные "
              "сочетания.")
        .arg(status["conflict_count"].toInt())
        .arg(first["device_name"].toString(), first["action_label"].toString());
  }
  if (!status["action_error"].toString().isEmpty())
    return tr("Не удалось выполнить действие. Проверьте доступность устройства или сценария и "
              "войдите снова при "
              "необходимости.");
  return {};
}
