import QtQuick
import QtQuick.Window
import QtTest
import YandexHomeDesktop.Pages as Pages

TestCase {
  id: testCase
  name: "ScenarioKeybinds"
  width: 350
  height: 460
  visible: true
  when: windowShown

  Component {
    id: scenarioPage
    Pages.ScenariosPage {
      height: 460
      viewModel: scenariosViewModel
      keybindSettings: keybindSettingsModel
      showKeybinds: settings.showKeybinds
    }
  }

  function init() {
    failOnWarning(/.?/);
    deviceTestApi.reset();
    settings.showKeybinds = true;
    scenarioTestApi.SetRows([
      { id: "evening", name: "A very long scenario name for all the lights in the house", is_active: true },
      { id: "other", name: "A very long scenario name for all the lights in the house", is_active: true },
      { id: "inactive", name: "Inactive scenario", is_active: false }
    ]);
    scenariosViewModel.Refresh();
  }

  function cleanup() {
    testCase.Window.window.width = 600;
    testCase.Window.window.height = 600;
  }

  function openPage(width) {
    const page = createTemporaryObject(scenarioPage, testCase, { width: width ?? 350 });
    waitForRendering(page);
    return page;
  }

  function badge(page, id) {
    const row = findChild(page, "scenarioDelegate_" + id);
    verify(row !== null);
    return findChild(row, "scenarioKeybind");
  }

  function record(button, key, modifiers) {
    mouseClick(button);
    tryCompare(button.editor, "opened", true);
    tryCompare(findChild(button.editor.contentItem, "keybindRecorder"), "activeFocus", true);
    keyClick(key, modifiers ?? Qt.NoModifier);
  }

  function save(button) {
    const saveButton = findChild(button.editor.contentItem, "keybindSave");
    verify(saveButton.enabled, button.editor.errorMessage);
    mouseClick(saveButton);
    tryCompare(button.editor, "visible", false);
  }

  function test_editPersistClearAndRunByStableId_data() {
    return [ { tag: "light", viewportWidth: 320, theme: 0 },
             { tag: "dark", viewportWidth: 350, theme: 1 } ];
  }

  function test_editPersistClearAndRunByStableId(data) {
    testCase.Window.window.width = data.viewportWidth;
    testCase.Window.window.height = 460;
    themes.SetTheme(data.theme);
    let page = openPage(data.viewportWidth);
    let first = badge(page, "evening");
    record(first, Qt.Key_F6, Qt.ControlModifier | Qt.AltModifier);
    save(first);
    const combination = first.combination;
    verify(combination !== 0);
    compare(first.bindingTarget.scenario_id, "evening");
    compare(badge(page, "other").sequence, "");
    const row = findChild(page, "scenarioDelegate_evening");
    const label = findChild(row, "scenarioName");
    verify(label.x + label.width <= first.x);
    verify(first.x + first.width <= findChild(row, "scenarioStart").x);
    compare(scenarioTestApi.executionCount, 0);
    if (settingsPreviewPath.length > 0)
      grabImage(testCase.Window.window.contentItem).save(settingsPreviewPath + "-scenario-keybind-" + data.tag + ".png");
    page.destroy();
    page = openPage(data.viewportWidth);
    first = badge(page, "evening");
    compare(first.combination, combination);
    record(first, Qt.Key_F8);
    keyClick(Qt.Key_Escape);
    tryCompare(first.editor, "visible", false);
    compare(first.combination, combination);
    const play = findChild(findChild(page, "scenarioDelegate_other"), "scenarioStart");
    mouseClick(play);
    compare(scenarioTestApi.executionCount, 1);
    compare(scenarioTestApi.lastScenario, "other");
    record(first, Qt.Key_F8);
    mouseClick(findChild(first.editor.contentItem, "keybindClear"));
    save(first);
    compare(first.sequence, "");
    compare(scenarioTestApi.executionCount, 1);
  }

  function test_deviceAndScenarioConflictsAndInactiveEditing() {
    const deviceTarget = { device_id: "lamp", capability_type: "devices.capabilities.on_off",
      instance: "on", action: "toggle", value: "", scenario_id: "" };
    verify(keybindSettingsModel.Save(deviceTarget, Qt.Key_F6, "Desk lamp", "Power"));
    const page = openPage();
    const first = badge(page, "evening");
    record(first, Qt.Key_F6);
    verify(first.editor.errorMessage.indexOf("Desk lamp") >= 0);
    compare(findChild(first.editor.contentItem, "keybindSave").enabled, false);
    keyClick(Qt.Key_F7);
    save(first);
    const other = badge(page, "other");
    record(other, Qt.Key_F7);
    verify(other.editor.errorMessage.indexOf("A very long scenario") >= 0);
    compare(findChild(other.editor.contentItem, "keybindSave").enabled, false);
    keyClick(Qt.Key_Escape);
    tryCompare(other.editor, "visible", false);
    const inactive = badge(page, "inactive");
    record(inactive, Qt.Key_F8);
    save(inactive);
    compare(inactive.sequence, "F8");
    mouseClick(findChild(findChild(page, "scenarioDelegate_inactive"), "scenarioStart"));
    compare(scenarioTestApi.executionCount, 0);
    compare(deviceTestApi.actionCount, 0);
  }

  function test_hideAndAccountSwitchPreserveAssignmentsAndCloseEditor() {
    const page = openPage();
    const first = badge(page, "evening");
    record(first, Qt.Key_F6);
    save(first);
    record(first, Qt.Key_F7);
    settings.showKeybinds = false;
    tryCompare(first.editor, "visible", false);
    compare(first.visible, false);
    compare(first.sequence, "F6");
    settings.showKeybinds = true;
    record(first, Qt.Key_F7);
    deviceTestApi.SelectTestAccount("other-account");
    tryCompare(first.editor, "visible", false);
    compare(first.sequence, "");
    record(first, Qt.Key_F8);
    save(first);
    deviceTestApi.SelectTestAccount("test-account");
    compare(first.sequence, "F6");
    compare(scenarioTestApi.executionCount, 0);
  }
}
