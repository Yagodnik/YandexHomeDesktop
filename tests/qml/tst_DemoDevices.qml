import QtQuick
import QtQuick.Window
import QtTest
import YandexHomeDesktop.ViewModels
import YandexHomeDesktop.Pages as Pages

TestCase {
  id: testCase
  name: "DemoDevices"
  width: 350
  height: 460
  visible: true
  when: windowShown

  Component { id: pageComponent; Pages.DevicePage { height: 460 } }

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

  function test_demoDevicePages_data() {
    const rows = [];
    for (let i = 0; i < demoDevices.length; ++i) {
      for (let theme = 0; theme < 2; ++theme) {
        rows.push({ tag: demoDevices[i].id + (theme === 0 ? "-light" : "-dark"),
          info: demoDevices[i], theme: theme, viewportWidth: theme === 0 ? 320 : 350 });
      }
    }
    return rows;
  }

  function test_demoDevicePages(data) {
    testCase.Window.window.width = data.viewportWidth;
    testCase.Window.window.height = 460;
    themes.SetTheme(data.theme);
    const info = data.info;
    deviceViewModel.LoadDevice(info.id);
    const page = createTemporaryObject(pageComponent, testCase, { width: data.viewportWidth });
    verify(page !== null);
    deviceTestApi.ReplyFromFixture(info);
    deviceViewModel.StopPolling();
    compare(deviceViewModel.state, DeviceViewModel.Ready);
    compare(capabilitiesModel.count, info.capabilities.length);
    compare(propertiesModel.count, info.properties.length);
    waitForRendering(page);

    for (let i = 0; i < info.capabilities.length; ++i) {
      const capability = info.capabilities[i];
      switch (capability.type) {
        case "devices.capabilities.on_off": verify(findChild(page, "deviceOnAction") !== null); break;
        case "devices.capabilities.toggle": verify(findChild(page, "deviceToggleControl") !== null); break;
        case "devices.capabilities.range":
          compare(findChild(page, "deviceRangeSlider").visible, capability.parameters.random_access);
          break;
        case "devices.capabilities.mode": {
          const picker = findChild(page, "deviceModePicker");
          verify(picker !== null);
          picker.popup.open();
          tryCompare(picker.popup, "opened", true);
          picker.popup.close();
          break;
        }
        case "devices.capabilities.color_setting": {
          const control = findChild(page, "deviceColorControl");
          verify(control !== null);
          const parameters = capability.parameters;
          if (!parameters.color_model && !parameters.temperature_k) compare(control.currentTab, 1);
          if (parameters.color_scene) {
            control.currentTab = 1;
            verify(findChild(control, "sceneOption-" + parameters.color_scene.scenes[0].id) !== null);
          }
          break;
        }
        case "devices.capabilities.video_stream": {
          const control = findChild(page, "videoStreamControl");
          verify(control !== null);
          compare(control.streamSource.toString(), "");
          verify(findChild(control, "videoStreamPlayAction") !== null);
          break;
        }
        default: fail("Unexpected demo capability " + capability.type);
      }
    }
    for (let i = 0; i < info.properties.length; ++i) {
      verify(findChild(page, info.properties[i].type === "devices.properties.float"
        ? "floatPropertyControl" : "eventPropertyControl") !== null);
    }
    const pane = findChild(page, "deviceControlsPane");
    tryVerify(function() { return pane.contentHeight > pane.height; });
    pane.contentY = pane.contentHeight - pane.height;
    tryCompare(pane, "atYEnd", true);
    waitForRendering(page);
    compare(deviceTestApi.actionCount, 0);
    if (settingsPreviewPath.length > 0) {
      grabImage(page).save(settingsPreviewPath + "-demo-" + data.tag + ".png");
    }
  }
}
