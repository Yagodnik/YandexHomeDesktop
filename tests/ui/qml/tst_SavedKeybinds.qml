import QtQuick
import QtQuick.Window
import QtTest
import YandexHomeDesktop.Pages as Pages

TestCase {
  id: testCase
  name: "SavedKeybinds"
  width: 350
  height: 460
  visible: true
  when: windowShown

  Component { id: devicePage; Pages.DevicePage { height: 460 } }

  function init() {
    failOnWarning(/.?/);
    deviceViewModel.StopPolling();
    deviceTestApi.reset();
    settings.showKeybinds = true;
  }

  function cleanup() {
    deviceViewModel.StopPolling();
    testCase.Window.window.width = 600;
    testCase.Window.window.height = 600;
  }

  function openDevice(id, name, width) {
    deviceViewModel.ForgetDevice();
    deviceTestApi.deviceName = name;
    deviceTestApi.withCapability = true;
    deviceViewModel.LoadDevice(id);
    const page = createTemporaryObject(devicePage, testCase, { width: width ?? 350 });
    deviceTestApi.ReplyDevice(true);
    deviceViewModel.StopPolling();
    compare(deviceViewModel.deviceId, id);
    waitForRendering(page);
    return page;
  }

  function record(badge, key) {
    waitForRendering(badge);
    mouseClick(badge);
    tryCompare(badge.editor, "opened", true);
    tryCompare(findChild(badge.editor.contentItem, "keybindRecorder"), "activeFocus", true);
    keyClick(key);
  }

  function save(badge) {
    const button = findChild(badge.editor.contentItem, "keybindSave");
    compare(button.enabled, true, badge.editor.errorMessage);
    mouseClick(button);
    tryCompare(badge.editor, "visible", false);
  }

  function test_crossDevicePersistenceAndConflicts_data() {
    return [
      { tag: "light", viewportWidth: 320, theme: 0 },
      { tag: "dark", viewportWidth: 350, theme: 1 }
    ];
  }

  function test_crossDevicePersistenceAndConflicts(data) {
    testCase.Window.window.width = data.viewportWidth;
    testCase.Window.window.height = 460;
    themes.SetTheme(data.theme);
    let page = openDevice("lamp", "Desk lamp", data.viewportWidth);
    let badge = findChild(page, "capabilityKeybind");
    record(badge, Qt.Key_F6);
    save(badge);
    compare(badge.sequence, "F6");
    const lampTarget = Object.assign({}, badge.bindingTarget);
    page.destroy();

    page = openDevice("socket", "Reading socket", data.viewportWidth);
    badge = findChild(page, "capabilityKeybind");
    compare(badge.sequence, "");
    record(badge, Qt.Key_F6);
    verify(badge.editor.errorMessage.indexOf("Desk lamp") >= 0);
    compare(findChild(badge.editor.contentItem, "keybindSave").enabled, false);
    verify(badge.editor.y >= 0 && badge.editor.y + badge.editor.height <= 460);
    compare(keybindSettingsModel.Save(badge.bindingTarget, badge.editor.draftCombination, "Reading socket", "On/off"), false);
    compare(keybindSettingsModel.GetCombination(lampTarget), Qt.Key_F6);
    compare(badge.sequence, "");
    if (settingsPreviewPath.length > 0) {
      wait(100);
      grabImage(testCase.Window.window.contentItem).save(settingsPreviewPath + "-keybind-conflict-" + data.tag + ".png");
    }
    keyClick(Qt.Key_F7);
    compare(badge.editor.errorMessage, "");
    save(badge);
    page.destroy();

    page = openDevice("lamp", "Renamed lamp", data.viewportWidth);
    badge = findChild(page, "capabilityKeybind");
    compare(badge.sequence, "F6");
    mouseClick(badge);
    tryCompare(badge.editor, "opened", true);
    mouseClick(findChild(badge.editor.contentItem, "keybindClear"));
    save(badge);
    page.destroy();

    page = openDevice("socket", "Reading socket", data.viewportWidth);
    badge = findChild(page, "capabilityKeybind");
    compare(badge.sequence, "F7");
    record(badge, Qt.Key_F6);
    save(badge);
    compare(badge.sequence, "F6");
    compare(keybindSettingsModel.GetCombination(lampTarget), 0);
    compare(deviceTestApi.actionCount, 0);
  }

  function test_sameDeviceConflictAndHidingPreservesAssignment() {
    deviceTestApi.SetCapability("range", { instance: "brightness", value: 50 }, {
      instance: "brightness", random_access: true, range: { min: 0, max: 100, precision: 1 }, unit: "unit.percent"
    });
    let page = openDevice("lamp", "Desk lamp");
    let decrease = findChild(page, "rangeDecreaseKeybind");
    let increase = findChild(page, "rangeIncreaseKeybind");
    record(decrease, Qt.Key_F6);
    save(decrease);
    record(increase, Qt.Key_F6);
    verify(increase.editor.errorMessage.indexOf("Desk lamp") >= 0);
    compare(findChild(increase.editor.contentItem, "keybindSave").enabled, false);
    settings.showKeybinds = false;
    tryCompare(increase.editor, "visible", false);
    compare(decrease.sequence, "F6");
    compare(increase.sequence, "");
    page.destroy();
    settings.showKeybinds = true;
    page = openDevice("lamp", "Desk lamp");
    decrease = findChild(page, "rangeDecreaseKeybind");
    increase = findChild(page, "rangeIncreaseKeybind");
    compare(decrease.sequence, "F6");
    record(increase, Qt.Key_F7);
    save(increase);
    compare(deviceTestApi.actionCount, 0);
  }

  function test_recordingWaitsForShortcutRelease() {
    shortcutViewModel.SetEnabled(true);
    const page = openDevice("lamp", "Desk lamp");
    const badge = findChild(page, "capabilityKeybind");
    mouseClick(badge);
    tryCompare(badge.editor, "opened", true);
    compare(badge.editor.recordingReady, false);
    keyClick(Qt.Key_F6);
    compare(badge.editor.draftCombination, 0);
    keyClick(Qt.Key_Escape);
    tryCompare(badge.editor, "visible", false);
    mouseClick(badge);
    tryCompare(badge.editor, "opened", true);
    compare(findChild(badge.editor.contentItem, "keybindSave").enabled, false);
    shortcutViewModel.SetEnabled(false);
    tryCompare(badge.editor, "recordingReady", true);
    findChild(badge.editor.contentItem, "keybindRecorder").forceActiveFocus();
    keyClick(Qt.Key_F6);
    save(badge);
    compare(badge.sequence, "F6");
  }

  function test_accountSwitchClosesEditorAndSeparatesAssignments() {
    const page = openDevice("lamp", "Desk lamp");
    const badge = findChild(page, "capabilityKeybind");
    record(badge, Qt.Key_F6);
    save(badge);
    record(badge, Qt.Key_F7);
    deviceTestApi.SelectTestAccount("other-account");
    tryCompare(badge.editor, "visible", false);
    compare(badge.sequence, "");
    record(badge, Qt.Key_F6);
    save(badge);
    deviceTestApi.SelectTestAccount("");
    compare(badge.sequence, "");
    record(badge, Qt.Key_F8);
    compare(findChild(badge.editor.contentItem, "keybindSave").enabled, false);
    deviceTestApi.SelectTestAccount("test-account");
    tryCompare(badge.editor, "visible", false);
    compare(badge.sequence, "F6");
    compare(deviceTestApi.actionCount, 0);
  }
}
