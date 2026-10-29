import QtQuick
import QtQuick.Window
import QtTest
import YandexHomeDesktop.ViewModels
import YandexHomeDesktop.Pages as Pages
import YandexHomeDesktop.Components as Components

TestCase {
  id: testCase
  name: "PageComposition"
  width: 600
  height: 600
  visible: true
  when: windowShown

  Component { id: authPage; Pages.AuthPage { width: 350; height: 460 } }
  Component { id: mainPage; Pages.MainPage { width: 350; height: 460 } }
  Component { id: devicePage; Pages.DevicePage { width: 350; height: 460 } }
  Component { id: settingsPage; Pages.SettingsPage { width: 350; height: 460; restModel: restViewModel; transferModel: settingsTransferModel; shortcutModel: shortcutViewModel } }
  QtObject {
    id: mcpStub
    property bool running: false
    property bool busy: false
    property int port: 8767
    property string url: "http://127.0.0.1:8767/mcp"
    property string error: ""
    property int calls: 0
    function setEnabled(enabled) { ++calls; running = enabled; }
    function setPort(value) { port = value; }
  }

  ListModel { id: households }
  ListModel { id: scenarios }
  SignalSpy { id: householdSpy; signalName: "householdSelected" }
  SignalSpy { id: scenarioSpy; signalName: "scenarioRequested" }

  Component {
    id: pickerComponent
    Components.HouseholdPicker {
      width: 350
      height: 460
      title: "Choose home"
      sourceModel: households
      currentHousehold: "one"
      loading: false
    }
  }

  Component {
    id: scenariosComponent
    Components.ScenariosPane {
      width: 350
      height: 120
      sourceModel: scenarios
      emptyMessage: "No scenarios"
    }
  }

  function init() {
    failOnWarning(/.?/);
    authorizationService.resetCalls();
    deviceViewModel.StopPolling();
    deviceTestApi.reset();
    deviceViewModel.LoadDevice("test-device");
    platformService.resetCalls();
    settings.trayModeEnabled = false;
    settings.currentTheme = 0;
    settings.showKeybinds = true;
    themes.SetTheme(0);
    households.clear();
    scenarios.clear();
    householdSpy.clear();
    scenarioSpy.clear();
  }

  function cleanup() {
    deviceViewModel.StopPolling();
    testCase.Window.window.width = 600;
    testCase.Window.window.height = 600;
  }

  function test_authAction() {
    const page = createTemporaryObject(authPage, testCase);
    verify(page !== null);
    const action = findChild(page, "signInAction");
    verify(action !== null);
    mouseClick(action);
    compare(authorizationService.lastCall, "AttemptAuthorization");
    compare(authorizationService.callCount, 1);
  }

  function test_deviceRetryAndInitialization() {
    const page = createTemporaryObject(devicePage, testCase);
    verify(page !== null);
    const states = findChild(page, "deviceStates");
    compare(states.currentIndex, 0);
    deviceTestApi.ReplyDevice(false);
    compare(states.currentIndex, 1);
    const dialog = findChild(page, "deviceErrorDialog");
    tryCompare(dialog, "opacity", 1);
    mouseClick(findChild(dialog, "errorDismissAction"));
    tryCompare(dialog, "visible", false);

    const action = findChild(page, "retryAction");
    mouseClick(action);
    compare(deviceTestApi.requestCount, 2);
    compare(deviceTestApi.deviceId, "test-device");
    compare(states.currentIndex, 0);
    deviceTestApi.ReplyDevice(true);
    compare(states.currentIndex, 2);
  }

  function test_deviceReadyBeforePageCreation() {
    // Loading starts on the device row before navigation creates the page.
    deviceTestApi.ReplyDevice(true);
    const page = createTemporaryObject(devicePage, testCase);
    verify(page !== null);
    compare(findChild(page, "deviceStates").currentIndex, DeviceViewModel.Ready);
  }

  function test_deviceCommandThroughRealDelegate() {
    const page = createTemporaryObject(devicePage, testCase);
    verify(page !== null);
    deviceTestApi.withCapability = true;
    deviceTestApi.ReplyDevice(true);
    deviceViewModel.StopPolling();
    tryVerify(function() { return findChild(page, "deviceOnAction") !== null; });
    const onAction = findChild(page, "deviceOnAction");
    tryVerify(function() { return onAction.visible && onAction.width > 0 && onAction.height > 0; });
    waitForRendering(onAction);
    mouseClick(onAction);
    compare(deviceTestApi.actionCount, 1);
    compare(deviceTestApi.actionDeviceId, "test-device");
    compare(deviceTestApi.actionState.instance, "on");
    compare(deviceTestApi.actionState.value, true);
    compare(capabilitiesModel.GetState(0).value, true);
    mouseClick(findChild(page, "deviceOffAction"));
    compare(deviceTestApi.actionCount, 2);
    compare(deviceTestApi.actionState.value, false);
    compare(capabilitiesModel.GetState(0).value, false);
  }

  function test_mainHouseholdPicker() {
    const page = createTemporaryObject(mainPage, testCase);
    verify(page !== null);
    const picker = findChild(page, "householdPicker");
    compare(picker.opened, false);
    mouseClick(findChild(page, "householdSelector"));
    compare(picker.opened, true);
    mouseClick(findChild(picker, "householdBackdrop"), 10, 10);
    compare(picker.opened, false);
  }

  function test_keybindPreviewDoesNotControlDevice() {
    const page = createTemporaryObject(devicePage, testCase);
    deviceTestApi.withCapability = true;
    deviceTestApi.ReplyDevice(true);
    deviceViewModel.StopPolling();
    const keybind = findChild(page, "capabilityKeybind");
    verify(keybind !== null);
    tryCompare(keybind, "visible", true);
    compare(keybind.sequence, "");
    waitForRendering(keybind);
    mouseClick(keybind);
    tryCompare(keybind.editor, "opened", true);
    const recorder = findChild(keybind.editor.contentItem, "keybindRecorder");
    tryCompare(recorder, "activeFocus", true);
    keyClick(Qt.Key_K, Qt.ControlModifier);
    const sequence = keybindSettingsModel.DisplayCombination(keybindSettingsModel.MakeCombination(Qt.Key_K, Qt.ControlModifier));
    compare(keybind.editor.draftSequence, sequence);
    compare(deviceTestApi.actionCount, 0);
    mouseClick(findChild(keybind.editor.contentItem, "keybindSave"));
    tryCompare(keybind.editor, "visible", false);
    compare(keybind.sequence, sequence);

    // Recorded keys are presentation only, including after closing the editor.
    page.forceActiveFocus();
    keyClick(Qt.Key_K, Qt.ControlModifier);
    compare(deviceTestApi.actionCount, 0);
    if (settingsPreviewPath.length > 0) {
      wait(100); // Let the control's icon effects settle before capturing it.
      grabImage(page).save(settingsPreviewPath + "-keybind-light.png");
      themes.SetTheme(1);
      waitForRendering(page);
      wait(100);
      grabImage(page).save(settingsPreviewPath + "-keybind-dark.png");
    }

    mouseClick(keybind);
    tryCompare(recorder, "activeFocus", true);
    keyClick(Qt.Key_L);
    keyClick(Qt.Key_Escape);
    tryCompare(keybind.editor, "visible", false);
    compare(keybind.sequence, sequence);

    mouseClick(keybind);
    tryCompare(recorder, "activeFocus", true);
    mouseClick(findChild(keybind.editor.contentItem, "keybindClear"));
    compare(keybind.editor.draftSequence, "");
    mouseClick(findChild(keybind.editor.contentItem, "keybindSave"));
    compare(keybind.sequence, "");
    compare(deviceTestApi.actionCount, 0);
  }

  function test_keybindVisibilitySetting() {
    const device = createTemporaryObject(devicePage, testCase);
    deviceTestApi.withCapability = true;
    deviceTestApi.ReplyDevice(true);
    deviceViewModel.StopPolling();
    const keybind = findChild(device, "capabilityKeybind");
    tryCompare(keybind, "visible", true);

    const page = createTemporaryObject(settingsPage, testCase, { height: 600 });
    const row = findChild(page, "keybindsSettingRow");
    const toggle = findChild(row, "settingSwitch");
    waitForRendering(toggle);
    mouseClick(toggle);
    compare(settings.showKeybinds, false);
    compare(keybind.visible, false);
    compare(platformService.callCount, 0);
    mouseClick(toggle);
    compare(settings.showKeybinds, true);
    compare(keybind.visible, true);
    compare(deviceTestApi.actionCount, 0);
  }

  function test_keybindEditorLayout_data() {
    return [
      { tag: "narrow-light", viewportWidth: 320, theme: 0 },
      { tag: "narrow-dark", viewportWidth: 320, theme: 1 },
      { tag: "default-light", viewportWidth: 350, theme: 0 },
      { tag: "default-dark", viewportWidth: 350, theme: 1 }
    ];
  }

  function test_keybindEditorAnimation() {
    const page = createTemporaryObject(devicePage, testCase);
    deviceTestApi.withCapability = true;
    deviceTestApi.ReplyDevice(true);
    deviceViewModel.StopPolling();
    const keybind = findChild(page, "capabilityKeybind");
    waitForRendering(keybind);
    mouseClick(keybind);
    const editor = keybind.editor;
    verify(editor.visible);
    compare(editor.opened, false);
    verify(editor.opacity < 1);
    tryCompare(editor, "opened", true);
    fuzzyCompare(editor.opacity, 1, 0.01);
    fuzzyCompare(editor.scale, 1, 0.01);
    keyClick(Qt.Key_F6);
    keyClick(Qt.Key_Escape);
    verify(editor.visible, "Keep the dialog visible while its exit animation runs");
    tryCompare(editor, "visible", false);
    compare(keybind.sequence, "");
    mouseClick(keybind);
    tryCompare(editor, "opened", true);
    compare(editor.draftSequence, "");
    keyClick(Qt.Key_F7);
    mouseClick(findChild(editor.contentItem, "keybindSave"));
    tryCompare(editor, "visible", false);
    compare(keybind.sequence, "F7");
    compare(deviceTestApi.actionCount, 0);
  }

  function test_keybindEditorLayout(data) {
    testCase.Window.window.width = data.viewportWidth;
    testCase.Window.window.height = 460;
    themes.SetTheme(data.theme);
    const page = createTemporaryObject(devicePage, testCase, { width: data.viewportWidth });
    deviceTestApi.withCapability = true;
    deviceTestApi.ReplyDevice(true);
    deviceViewModel.StopPolling();
    const keybind = findChild(page, "capabilityKeybind");
    waitForRendering(keybind);
    if (settingsPreviewPath.length > 0) {
      wait(100);
      grabImage(page).save(settingsPreviewPath + "-keybind-" + data.tag + ".png");
    }
    mouseClick(keybind);
    tryCompare(keybind.editor, "opened", true);
    const editor = keybind.editor;
    verify(editor.x >= 0 && editor.y >= 0);
    verify(editor.x + editor.width <= data.viewportWidth);
    verify(editor.y + editor.height <= 460);
    const recorder = findChild(editor.contentItem, "keybindRecorder");
    tryCompare(recorder, "activeFocus", true);
    keyClick(Qt.Key_Shift);
    compare(editor.draftSequence, "");
    keyClick(Qt.Key_F5);
    compare(editor.draftSequence, "F5");
    if (settingsPreviewPath.length > 0) {
      wait(100);
      grabImage(testCase.Window.window.contentItem).save(settingsPreviewPath + "-keybind-editor-" + data.tag + ".png");
    }
    keyClick(Qt.Key_Tab);
    verify(!recorder.activeFocus, "Tab must reach the dialog actions");
    compare(editor.draftSequence, "F5");
    settings.showKeybinds = false;
    tryCompare(editor, "visible", false);
    compare(deviceTestApi.actionCount, 0);
  }

  function test_householdSelection() {
    households.append({ name: "First home", householdId: "one" });
    households.append({ name: "Second home", householdId: "two" });
    const picker = createTemporaryObject(pickerComponent, testCase);
    verify(picker !== null);
    householdSpy.target = picker;
    picker.open();
    wait(350); // Wait for the existing sheet animation before clicking a row.
    const row = findChild(picker, "household-two");
    verify(row !== null);
    mouseClick(row);
    compare(householdSpy.count, 1);
    compare(householdSpy.signalArguments[0][0], "two");
  }

  function test_settingsScrollingAndActions() {
    const page = createTemporaryObject(settingsPage, testCase, { height: 120 });
    verify(page !== null);
    const content = findChild(page, "settingsContent");
    const card = findChild(page, "basicSettings");
    tryVerify(function() { return content.contentHeight > content.height; });
    content.contentY = content.contentHeight - content.height;
    const advanced = findChild(page, "advancedSettings");
    fuzzyCompare(advanced.mapToItem(content, 0, advanced.height).y, content.height, 0.1);

    // Enlarge the viewport so both settings controls are visible for interaction.
    page.height = 460;
    content.contentY = 0;
    const traySwitch = findChild(page, "settingSwitch");
    mouseClick(traySwitch);
    compare(settings.trayModeEnabled, true);
    compare(platformService.lastCall, "ShowOnlyInTray");
    mouseClick(traySwitch);
    compare(settings.trayModeEnabled, false);
    compare(platformService.lastCall, "ShowAsApp");

    const choice = findChild(page, "settingChoice");
    mouseClick(choice);
    tryCompare(choice.popup, "visible", true);
    mouseClick(choice.popup.contentItem, 10, 48);
    compare(settings.currentTheme, 1);
    compare(page.color, themes.background);
  }

  function test_settingsCardsShareInsets_data() {
    return [{ tag: "narrow-light", viewportWidth: 320, theme: 0 },
            { tag: "dark", viewportWidth: 350, theme: 1 }];
  }

  function test_settingsCardsShareInsets(data) {
    themes.SetTheme(data.theme);
    const page = createTemporaryObject(settingsPage, testCase, { width: data.viewportWidth });
    const basic = findChild(page, "basicSettings");
    const shortcuts = findChild(page, "shortcutSettings");
    const advanced = findChild(page, "advancedSettings");
    const transfer = findChild(page, "settingsTransferCard");
    const rows = [findChild(basic, "keybindsSettingRow"),
                  findChild(shortcuts, "shortcutsEnabledRow"),
                  findChild(advanced, "restSettingRow"),
                  findChild(transfer, "exportSettingsRow")];
    const cards = [basic, shortcuts, advanced, transfer];
    waitForRendering(page);
    const switches = rows.slice(0, 3).map(function(row) { return findChild(row, "settingSwitch"); });
    for (let i = 0; i < rows.length; ++i) {
      compare(rows[i].width, cards[i].width);
      compare(rows[i].mapToItem(cards[i], 0, 0).x, 0);
    }
    compare(rows[1].mapToItem(shortcuts, 0, 0).y, 0);
    compare(rows[2].mapToItem(advanced, 0, 0).y, 0);
    for (let i = 1; i < switches.length; ++i) {
      compare(switches[i].mapToItem(cards[i], 0, 0).x,
              switches[0].mapToItem(cards[0], 0, 0).x);
    }
    const content = findChild(page, "settingsContent");
    content.contentY = Math.max(0, content.contentHeight - content.height);
    if (settingsPreviewPath.length > 0) {
      waitForRendering(page);
      grabImage(page).save(settingsPreviewPath + "-insets-" + data.tag + ".png");
    }
  }

  function test_restToggleReachesService() {
    restTestServer.SetRunning(false);
    restViewModel.Refresh();
    tryCompare(restViewModel, "busy", false);
    tryCompare(restViewModel, "enabled", false);
    const page = createTemporaryObject(settingsPage, testCase, { height: 600 });
    verify(page !== null);
    const content = findChild(page, "settingsContent");
    content.contentY = content.contentHeight - content.height;
    const row = findChild(page, "restSettingRow");
    const toggle = findChild(row, "settingSwitch");
    verify(toggle !== null);
    // Supply an existing local daemon; enable reuses it through the real controller.
    restTestServer.SetRunning(true);
    mouseClick(toggle);
    tryCompare(restViewModel, "enabled", true);
    tryCompare(restViewModel, "busy", false);
    const previous = restTestServer.disableCount;
    if (settingsPreviewPath.length > 0) {
      page.height = 460;
      content.contentY = content.contentHeight - content.height;
      wait(100);
      grabImage(page).save(settingsPreviewPath + "-enabled.png");
    }
    mouseClick(toggle);
    tryCompare(restTestServer, "disableCount", previous + 1);
    tryCompare(restViewModel, "enabled", false);
    if (settingsPreviewPath.length > 0) {
      wait(100);
      grabImage(page).save(settingsPreviewPath + "-disabled.png");
    }
  }

  function test_mcpToggleReachesModel() {
    mcpStub.running = false;
    mcpStub.calls = 0;
    mcpStub.port = 8767;
    const page = createTemporaryObject(settingsPage, testCase, { height: 460, mcpModel: mcpStub });
    const content = findChild(page, "settingsContent");
    const card = findChild(page, "mcpSettings");
    tryVerify(function() { return card.visible && content.contentHeight > content.height; });
    content.contentY = content.contentHeight - content.height;
    waitForRendering(card);
    const portInput = findChild(card, "serverPortInput");
    verify(portInput.enabled);
    portInput.forceActiveFocus();
    portInput.text = "9876";
    keyClick(Qt.Key_Return);
    compare(mcpStub.port, 9876);
    portInput.text = "0";
    page.forceActiveFocus();
    compare(portInput.text, "9876");
    compare(mcpStub.port, 9876);
    const toggle = findChild(findChild(card, "restSettingRow"), "settingSwitch");
    mouseClick(toggle);
    compare(mcpStub.calls, 1);
    compare(mcpStub.running, true);
    verify(!portInput.enabled);
    if (settingsPreviewPath.length > 0) grabImage(page).save(settingsPreviewPath + "-mcp-port.png");
    mouseClick(toggle);
    compare(mcpStub.calls, 2);
    compare(mcpStub.running, false);
  }

  function test_restPortReachesModel() {
    restViewModel.SetEnabled(false);
    tryCompare(restViewModel, "busy", false);
    tryCompare(restViewModel, "running", false);
    const original = restViewModel.port;
    const page = createTemporaryObject(settingsPage, testCase, { height: 460 });
    const content = findChild(page, "settingsContent");
    const card = findChild(page, "advancedSettings");
    content.contentY = content.contentHeight - content.height;
    waitForRendering(card);
    const input = findChild(card, "serverPortInput");
    verify(input.enabled);
    input.forceActiveFocus();
    input.text = "9875";
    keyClick(Qt.Key_Return);
    compare(restViewModel.port, 9875);
    restViewModel.SetPort(original);
  }

  function test_shortcutSettingsReachController() {
    shortcutViewModel.SetEnabled(false);
    tryCompare(shortcutViewModel, "busy", false);
    const page = createTemporaryObject(settingsPage, testCase, { height: 460 });
    const content = findChild(page, "settingsContent");
    const card = findChild(page, "shortcutSettings");
    tryVerify(function() { return card.y > 0 && content.contentHeight > content.height; });
    content.contentY = Math.min(card.y, content.contentHeight - content.height);
    waitForRendering(card);
    const toggle = findChild(findChild(card, "shortcutsEnabledRow"), "settingSwitch");
    mouseClick(toggle);
    tryCompare(shortcutViewModel, "enabled", true);
    const background = findChild(findChild(card, "shortcutsBackgroundRow"), "settingSwitch");
    mouseClick(background);
    compare(shortcutViewModel.background, true);
    mouseClick(background);
    compare(shortcutViewModel.background, false);
    if (settingsPreviewPath.length > 0) grabImage(page).save(settingsPreviewPath + "-shortcuts.png");
    mouseClick(toggle);
    tryCompare(shortcutViewModel, "enabled", false);
    tryCompare(shortcutViewModel, "busy", false);
  }

  function test_scenariosScrollingAndAction() {
    for (let i = 0; i < 30; ++i) {
      scenarios.append({ name: "Scenario " + i, scenario_id: "scenario-" + i,
        is_active: true, is_waiting_response: false });
    }
    const pane = createTemporaryObject(scenariosComponent, testCase);
    verify(pane !== null);
    scenarioSpy.target = pane;
    const scrollBar = findChild(pane, "scenariosScrollBar");
    tryVerify(function() { return scrollBar.view.contentHeight > pane.height; });
    mouseClick(findChild(pane, "scenarioStart"));
    compare(scenarioSpy.count, 1);
    compare(scenarioSpy.signalArguments[0][0], "scenario-0");

    scrollBar.position = 0.5;
    tryVerify(function() { return scrollBar.view.contentY > 0; });
    scenarios.clear();
    compare(pane.empty, true);
    compare(scrollBar.visible, false);
  }
}
