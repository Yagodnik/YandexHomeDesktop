#include "QHotkeyRegistrar.h"

bool QHotkeyRegistrar::Register(int combination, std::function<void()> activated) {
  auto hotkey = new QHotkey(QKeySequence(QKeyCombination::fromCombined(combination)), true, this);
  if (!hotkey->isRegistered()) {
    delete hotkey;
    return false;
  }
#ifdef Q_OS_MACOS
  // Carbon can repeat presses. Windows already uses MOD_NOREPEAT, and its
  // release polling follows only the most recently activated key.
  const auto held = std::make_shared<bool>(false);
  connect(hotkey, &QHotkey::activated, this, [held, activated = std::move(activated)] {
    if (std::exchange(*held, true))
      return;
    activated();
  });
  connect(hotkey, &QHotkey::released, this, [held] { *held = false; });
#else
  connect(hotkey, &QHotkey::activated, this, std::move(activated));
#endif
  hotkeys_.append(hotkey);
  return true;
}

bool QHotkeyRegistrar::Clear() {
  const auto hotkeys = std::exchange(hotkeys_, {});
  for (auto* hotkey : hotkeys) {
    if (hotkey->isRegistered() && !hotkey->setRegistered(false))
      hotkeys_.append(hotkey);
    else
      delete hotkey;
  }
  return hotkeys_.isEmpty();
}
