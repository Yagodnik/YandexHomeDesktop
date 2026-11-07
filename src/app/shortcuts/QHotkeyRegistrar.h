#pragma once
#include "services/ShortcutRuntime.h"
#include <QHotkey>
#include <QHash>

class QHotkeyRegistrar final : public QObject, public IShortcutRegistrar {
public:
  using QObject::QObject;
  ~QHotkeyRegistrar() override {
    Clear();
  }
  bool Register(int combination, std::function<void()> activated) override;
  bool Clear() override;

private:
  QList<QHotkey*> hotkeys_;
};
