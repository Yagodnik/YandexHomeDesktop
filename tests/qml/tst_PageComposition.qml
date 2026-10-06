import QtQuick
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
  Component { id: settingsPage; Pages.SettingsPage { width: 350; height: 460 } }

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
    themes.SetTheme(0);
    households.clear();
    scenarios.clear();
    householdSpy.clear();
    scenarioSpy.clear();
  }

  function cleanup() {
    deviceViewModel.StopPolling();
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
    fuzzyCompare(card.mapToItem(content, 0, card.height).y, content.height, 0.1);

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
