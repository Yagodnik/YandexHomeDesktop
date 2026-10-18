import QtQuick
import QtQuick.Window
import QtTest
import YandexHomeDesktop.Pages as Pages

TestCase {
  id: testCase
  name: "CapabilityKeybinds"
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

  function loadCapability(data, kind, state, parameters) {
    testCase.Window.window.width = data.viewportWidth;
    testCase.Window.window.height = 460;
    themes.SetTheme(data.theme);
    // Keep another capability first to exercise the device row identity.
    deviceTestApi.withCapability = true;
    deviceTestApi.SetCapability(kind, state, parameters);
    deviceViewModel.LoadDevice("test-device");
    const page = createTemporaryObject(devicePage, testCase, { width: data.viewportWidth });
    verify(page !== null);
    deviceTestApi.ReplyDevice(true);
    deviceViewModel.StopPolling();
    waitForRendering(page);
    return page;
  }

  function reveal(page, item) {
    verify(item !== null);
    const pane = findChild(page, "deviceControlsPane");
    const top = item.mapToItem(pane.contentItem, 0, 0).y;
    if (top < pane.contentY) pane.contentY = top;
    if (top + item.height > pane.contentY + pane.height)
      pane.contentY = top + item.height - pane.height;
    pane.contentY = Math.max(0, Math.min(pane.contentY, pane.contentHeight - pane.height));
    waitForRendering(item);
  }

  function assignKey(badge, key, expected) {
    verify(badge.visible);
    waitForRendering(badge);
    mouseClick(badge);
    tryCompare(badge.editor, "opened", true);
    tryCompare(findChild(badge.editor.contentItem, "keybindRecorder"), "activeFocus", true);
    keyClick(key);
    mouseClick(findChild(badge.editor.contentItem, "keybindSave"));
    tryCompare(badge.editor, "visible", false);
    compare(badge.sequence, expected);
  }

  function snapshot(item, suffix) {
    if (settingsPreviewPath.length > 0) {
      wait(100);
      grabImage(item).save(settingsPreviewPath + "-" + suffix + ".png");
    }
  }

  function test_rangeBindings_data() {
    return [
      { tag: "slider-light", viewportWidth: 320, theme: 0, randomAccess: true },
      { tag: "slider-dark", viewportWidth: 350, theme: 1, randomAccess: true },
      { tag: "relative-light", viewportWidth: 320, theme: 0, randomAccess: false },
      { tag: "relative-dark", viewportWidth: 350, theme: 1, randomAccess: false }
    ];
  }

  function test_rangeBindings(data) {
    const page = loadCapability(data, "range", { instance: "brightness", value: 50 }, {
      instance: "brightness", random_access: data.randomAccess, unit: "unit.percent",
      range: { min: 0, max: 100, precision: 1 }
    });
    const decrease = findChild(page, "rangeDecreaseKeybind");
    const increase = findChild(page, "rangeIncreaseKeybind");
    reveal(page, decrease);
    assignKey(decrease, Qt.Key_F6, "F6");
    assignKey(increase, Qt.Key_F7, "F7");
    compare(decrease.sequence, "F6");
    compare(deviceTestApi.actionCount, 0);
    compare(capabilitiesModel.GetState(1).value, 50);
    verify(data.randomAccess ? decrease.x + decrease.width < increase.x
      : increase.x + increase.width < decrease.x);
    const control = decrease.parent;
    const originalHeight = control.height;
    settings.showKeybinds = false;
    compare(decrease.visible, false);
    compare(increase.visible, false);
    compare(control.height, originalHeight - 22);
    settings.showKeybinds = true;
    compare(decrease.sequence, "F6");
    compare(increase.sequence, "F7");
    snapshot(page, "keybind-" + data.tag);

    if (data.randomAccess) {
      const slider = findChild(page, "deviceRangeSlider");
      mouseClick(slider, slider.width * 0.8, slider.height / 2);
      compare(deviceTestApi.actionCount, 1);
      verify(deviceTestApi.actionState.value > 50);
    } else {
      mouseClick(findChild(page, "deviceRangeIncreaseAction"));
      compare(deviceTestApi.actionCount, 1);
      compare(deviceTestApi.actionState.value, 1);
      mouseClick(findChild(page, "deviceRangeDecreaseAction"));
      compare(deviceTestApi.actionCount, 2);
      compare(deviceTestApi.actionState.value, -1);
    }
    compare(deviceTestApi.actionDeviceId, "test-device");
    compare(deviceTestApi.actionState.instance, "brightness");
    compare(capabilitiesModel.GetState(0).value, false);
  }

  function test_colorAndSceneBindings_data() {
    return [
      { tag: "light", viewportWidth: 320, theme: 0 },
      { tag: "dark", viewportWidth: 350, theme: 1 }
    ];
  }

  function test_colorAndSceneBindings(data) {
    const page = loadCapability(data, "color", { instance: "rgb", value: 0 }, {
      color_model: "rgb", temperature_k: { min: 2700, max: 3400 },
      color_scene: { scenes: [{ id: "alice" }, { id: "party" }] }
    });
    const red = findChild(page, "colorOption-0");
    const coral = findChild(page, "colorOption-1");
    const temperature = findChild(page, "temperatureOption-2700");
    const redKey = findChild(red, "optionKeybind");
    reveal(page, redKey);
    assignKey(redKey, Qt.Key_F6, "F6");
    reveal(page, findChild(coral, "optionKeybind"));
    assignKey(findChild(coral, "optionKeybind"), Qt.Key_F7, "F7");
    reveal(page, findChild(temperature, "optionKeybind"));
    assignKey(findChild(temperature, "optionKeybind"), Qt.Key_F8, "F8");
    compare(redKey.sequence, "F6");
    compare(deviceTestApi.actionCount, 0);
    snapshot(page, "keybind-colors-" + data.tag);

    const modesTab = findChild(page, "colorTab-1");
    reveal(page, modesTab);
    mouseClick(modesTab);
    const alice = findChild(page, "sceneOption-alice");
    const party = findChild(page, "sceneOption-party");
    const aliceKey = findChild(alice, "optionKeybind");
    reveal(page, aliceKey);
    assignKey(aliceKey, Qt.Key_F9, "F9");
    assignKey(findChild(party, "optionKeybind"), Qt.Key_F10, "F10");
    compare(aliceKey.sequence, "F9");
    compare(deviceTestApi.actionCount, 0);
    snapshot(page, "keybind-scenes-" + data.tag);
    mouseClick(findChild(alice, "optionAction"));
    compare(deviceTestApi.actionCount, 1);
    compare(deviceTestApi.actionState.instance, "scene");
    compare(deviceTestApi.actionState.value, "alice");

    const colorsTab = findChild(page, "colorTab-0");
    reveal(page, colorsTab);
    mouseClick(colorsTab);
    const restoredRed = findChild(page, "colorOption-0");
    const restoredTemperature = findChild(page, "temperatureOption-2700");
    compare(findChild(restoredRed, "optionKeybind").sequence, "F6");
    compare(findChild(findChild(page, "colorOption-1"), "optionKeybind").sequence, "F7");
    compare(findChild(restoredTemperature, "optionKeybind").sequence, "F8");
    settings.showKeybinds = false;
    compare(findChild(restoredRed, "optionKeybind").visible, false);
    compare(restoredRed.height, 90);
    settings.showKeybinds = true;
    compare(findChild(restoredRed, "optionKeybind").sequence, "F6");
    reveal(page, restoredTemperature);
    mouseClick(findChild(restoredTemperature, "optionAction"));
    compare(deviceTestApi.actionState.instance, "temperature_k");
    compare(deviceTestApi.actionState.value, 2700);
    reveal(page, restoredRed);
    mouseClick(findChild(restoredRed, "optionAction"));
    compare(deviceTestApi.actionCount, 3);
    compare(deviceTestApi.actionState.instance, "rgb");
    compare(deviceTestApi.actionState.value, (242 << 16) | (89 << 8) | 91);
    compare(capabilitiesModel.GetState(0).value, false);
  }

  function test_modeBindings_data() {
    return [
      { tag: "light", viewportWidth: 320, theme: 0 },
      { tag: "dark", viewportWidth: 350, theme: 1 }
    ];
  }

  function test_modeBindings(data) {
    const page = loadCapability(data, "mode", { instance: "fan_speed", value: "auto" }, {
      instance: "fan_speed", modes: [{ value: "auto" }, { value: "eco" }, { value: "turbo" }]
    });
    const picker = findChild(page, "deviceModePicker");
    reveal(page, picker);
    mouseClick(picker);
    tryCompare(picker.popup, "opened", true);
    const autoKey = findChild(findChild(picker.popup.contentItem, "modeOption-auto"), "modeOptionKeybind");
    assignKey(autoKey, Qt.Key_F6, "F6");
    if (!picker.popup.visible) mouseClick(picker);
    const ecoKey = findChild(findChild(picker.popup.contentItem, "modeOption-eco"), "modeOptionKeybind");
    assignKey(ecoKey, Qt.Key_F7, "F7");
    compare(deviceTestApi.actionCount, 0);
    picker.popup.close();
    mouseClick(picker);
    tryCompare(picker.popup, "opened", true);
    compare(findChild(findChild(picker.popup.contentItem, "modeOption-auto"), "modeOptionKeybind").sequence, "F6");
    compare(findChild(findChild(picker.popup.contentItem, "modeOption-eco"), "modeOptionKeybind").sequence, "F7");
    snapshot(testCase.Window.window.contentItem, "keybind-modes-" + data.tag);
    settings.showKeybinds = false;
    compare(ecoKey.visible, false);
    compare(picker.popup.width, picker.width);
    const eco = findChild(picker.popup.contentItem, "modeOption-eco");
    waitForRendering(eco);
    mouseClick(findChild(eco, "modeOptionAction"));
    compare(deviceTestApi.actionCount, 1);
    compare(deviceTestApi.actionState.instance, "fan_speed");
    compare(deviceTestApi.actionState.value, "eco");
    compare(capabilitiesModel.GetState(0).value, false);
  }
}
