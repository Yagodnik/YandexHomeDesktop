import QtQuick
import QtQuick.Window
import QtQuick.Dialogs
import QtTest
import YandexHomeDesktop.Pages as Pages
import YandexHomeDesktop.Ui as UI

TestCase {
  id: testCase
  name: "SettingsTransfer"
  width: 350
  height: 460
  visible: true
  when: windowShown
  Component {
    id: settingsPage
    Pages.SettingsPage { height: 460; restModel: restViewModel; transferModel: settingsTransferModel; shortcutModel: shortcutViewModel }
  }
  Component {
    id: trayWindow
    UI.TrayWindow {
      width: 350
      height: 460
      visible: true
      trayModeEnabled: true
      property int activationChanges: 0
      onActiveChanged: ++activationChanges
      modalDialogOpen: trayPage.modalDialogOpen
      Pages.SettingsPage {
        id: trayPage
        objectName: "traySettingsPage"
        anchors.fill: parent
        restModel: restViewModel
        transferModel: settingsTransferModel
        shortcutModel: shortcutViewModel
      }
    }
  }
  Component {
    id: popupWindow
    UI.TrayWindow {
      width: 350
      height: 460
      visible: true
    }
  }
  Component {
    id: popupMainWindow
    UI.TrayWindow {
      width: 350
      height: 460
      visible: true
      notchModeEnabled: true
      popupMenuInHeader: main.popupMenuInHeader
      Pages.MainPage { id: main; objectName: "popupMainPage"; anchors.fill: parent }
    }
  }
  Component {
    id: popupDeviceWindow
    UI.TrayWindow {
      width: 350; height: 460; visible: true; trayModeEnabled: true
      popupQuitVisible: device.popupQuitVisible
      Pages.DevicePage { id: device; anchors.fill: parent }
    }
  }
  SignalSpy { id: quitSpy; signalName: "quitRequested" }

  function init() {
    failOnWarning(/.?/);
    deviceTestApi.reset();
    settings.currentTheme = 0;
    themes.SetTheme(0);
  }
  function cleanup() {
    settingsTransferModel.CancelImport();
    testCase.Window.window.width = 600;
    testCase.Window.window.height = 600;
  }
  function test_exportAndConfirmedImport_data() {
    return [{ tag: "light", viewportWidth: 320, theme: 0 },
            { tag: "dark", viewportWidth: 350, theme: 1 }];
  }
  function test_exportAndConfirmedImport(data) {
    testCase.Window.window.width = data.viewportWidth;
    testCase.Window.window.height = 460;
    themes.SetTheme(data.theme);
    const page = createTemporaryObject(settingsPage, testCase, { width: data.viewportWidth });
    const card = findChild(page, "settingsTransferCard");
    const heading = findChild(page, "settingsTransferHeading");
    const content = findChild(page, "settingsContent");
    waitForRendering(card);
    verify(heading.y + heading.height <= card.y);
    content.contentY = heading.y;
    waitForRendering(card);
    const exportDialog = findChild(page, "exportSettingsDialog");
    const importDialog = findChild(page, "importSettingsDialog");
    exportDialog.options = FileDialog.DontUseNativeDialog;
    importDialog.options = FileDialog.DontUseNativeDialog;
    if (settingsPreviewPath.length > 0) {
      grabImage(page).save(settingsPreviewPath + "-transfer-" + data.tag + ".png");
    }
    settings.currentTheme = 1;
    settings.showKeybinds = false;
    const target = { device_id: "lamp", capability_type: "devices.capabilities.on_off", instance: "on", action: "toggle", value: "" };
    verify(keybindSettingsModel.Save(target, Qt.Key_F6, "Desk lamp", "On/off"));
    const exportAction = findChild(findChild(page, "exportSettingsRow"), "settingActionButton");
    mouseClick(exportAction);
    tryCompare(exportDialog, "visible", true);
    exportDialog.selectedFile = settingsTransferTestFile;
    exportDialog.accepted();
    exportDialog.close();
    compare(settingsTransferModel.failed, false);
    settings.currentTheme = 0;
    themes.SetTheme(0);
    settings.showKeybinds = true;
    verify(keybindSettingsModel.Save(target, 0, "Desk lamp", "On/off"));
    const importAction = findChild(findChild(page, "importSettingsRow"), "settingActionButton");
    mouseClick(importAction);
    tryCompare(importDialog, "visible", true);
    importDialog.selectedFile = settingsTransferTestFile;
    importDialog.accepted();
    importDialog.close();
    const confirmation = findChild(page, "confirmSettingsImport");
    tryCompare(confirmation, "opened", true);
    compare(settings.currentTheme, 0);
    verify(confirmation.y >= 0 && confirmation.y + confirmation.height <= 460);
    mouseClick(findChild(confirmation.contentItem, "cancelSettingsImportAction"));
    tryCompare(confirmation, "visible", false);
    compare(settings.currentTheme, 0);
    compare(keybindSettingsModel.GetCombination(target), 0);
    verify(settingsTransferModel.PrepareImport(settingsTransferTestFile));
    tryCompare(confirmation, "opened", true);
    if (settingsPreviewPath.length > 0) {
      grabImage(testCase.Window.window.contentItem).save(settingsPreviewPath + "-transfer-confirm-" + data.tag + ".png");
    }
    mouseClick(findChild(confirmation.contentItem, "confirmSettingsImportAction"));
    tryCompare(confirmation, "visible", false);
    compare(settings.currentTheme, 1);
    compare(settings.showKeybinds, false);
    compare(keybindSettingsModel.GetCombination(target), Qt.Key_F6);
    compare(settingsTransferModel.failed, false);
    compare(deviceTestApi.actionCount, 0);
  }
  function test_unavailableAndFileError() {
    const page = createTemporaryObject(settingsPage, testCase, { width: 350 });
    deviceTestApi.SelectTestAccount("");
    const exportAction = findChild(findChild(page, "exportSettingsRow"), "settingActionButton");
    const importAction = findChild(findChild(page, "importSettingsRow"), "settingActionButton");
    compare(exportAction.enabled, false);
    compare(importAction.enabled, false);
    deviceTestApi.SelectTestAccount("test-account");
    compare(exportAction.enabled, true);
    compare(settingsTransferModel.PrepareImport(settingsTransferTestFile + ".missing"), false);
    compare(settingsTransferModel.failed, true);
    verify(findChild(page, "settingsTransferStatus").text.length > 0);
  }

  function test_popupMenuAlignedWithHomeSwitcher() {
    const window = createTemporaryObject(popupMainWindow, null);
    verify(window !== null);
    const menu = findChild(window.contentItem, "popupQuitButton");
    const selector = findChild(window.contentItem, "householdSelector");
    verify(menu.visible);
    compare(menu.contentItem.text, "×");
    compare(menu.foregroundColor.toString(), themes.inactive.toString());
    tryCompare(window.contentItem, "width", 350);
    const menuPosition = menu.mapToItem(window.contentItem, 0, 0);
    const selectorPosition = selector.mapToItem(window.contentItem, 0, 0);
    compare(menuPosition.y + menu.height / 2, selectorPosition.y + selector.height / 2);
    verify(selectorPosition.x + selector.width < menuPosition.x, `selector ${selectorPosition.x} + ${selector.width}, menu ${menuPosition.x}`);
    compare(menuPosition.x + menu.width, window.width - 12);
  }

  function test_popupModesQuitAndHide_data() {
    return [{ tag: "tray", tray: true, notch: false },
            { tag: "notch", tray: false, notch: true },
            { tag: "notch-and-tray-fallback", tray: true, notch: true }];
  }

  function test_popupModesQuitAndHide(data) {
    const window = createTemporaryObject(popupWindow, null, {
      trayModeEnabled: data.tray, notchModeEnabled: data.notch
    });
    verify(window !== null);
    quitSpy.target = window;
    quitSpy.clear();
    const button = findChild(window.contentItem, "popupQuitButton");
    verify(button.visible);
    window.requestActivate();
    tryCompare(window, "active", true);
    mouseClick(button);
    compare(quitSpy.count, 1);
    window.modalDialogOpen = true;
    testCase.Window.window.requestActivate();
    tryCompare(window, "active", false);
    compare(window.visible, true);
    window.modalDialogOpen = false;
    window.requestActivate();
    tryCompare(window, "active", true);
    testCase.Window.window.requestActivate();
    tryCompare(window, "visible", false);
    quitSpy.target = null;
  }

  function test_popupQuitHiddenInNormalMode() {
    const window = createTemporaryObject(popupWindow, null);
    verify(window !== null);
    compare(findChild(window.contentItem, "popupQuitButton").visible, false);
    const deviceWindow = createTemporaryObject(popupDeviceWindow, null);
    verify(deviceWindow !== null);
    compare(findChild(deviceWindow.contentItem, "popupQuitButton").visible, false);
  }

  function test_trayWindowSurvivesDialogs() {
    const window = createTemporaryObject(trayWindow, null);
    verify(window !== null);
    const page = findChild(window.contentItem, "traySettingsPage");
    const exportDialog = findChild(page, "exportSettingsDialog");
    const importDialog = findChild(page, "importSettingsDialog");
    exportDialog.options = FileDialog.DontUseNativeDialog;
    importDialog.options = FileDialog.DontUseNativeDialog;
    const content = findChild(page, "settingsContent");
    content.contentY = findChild(page, "settingsTransferHeading").y;
    waitForRendering(page);
    mouseClick(findChild(findChild(page, "exportSettingsRow"), "settingActionButton"));
    tryCompare(exportDialog, "visible", true);
    compare(window.modalDialogOpen, true);
    testCase.Window.window.requestActivate();
    tryCompare(window, "active", false);
    compare(window.visible, true);
    exportDialog.selectedFile = settingsTransferTestFile;
    exportDialog.accepted();
    exportDialog.close();
    compare(settingsTransferModel.failed, false);
    tryCompare(window, "modalDialogOpen", false);
    compare(window.visible, true);
    content.contentY = findChild(page, "settingsTransferHeading").y;
    waitForRendering(page);
    const importButton = findChild(findChild(page, "importSettingsRow"), "settingActionButton");
    mouseClick(importButton);
    tryCompare(importDialog, "visible", true);
    compare(window.modalDialogOpen, true);
    testCase.Window.window.requestActivate();
    tryCompare(window, "active", false);
    compare(window.visible, true);
    importDialog.selectedFile = settingsTransferTestFile;
    importDialog.accepted();
    importDialog.close();
    const confirmation = findChild(page, "confirmSettingsImport");
    tryCompare(confirmation, "opened", true);
    compare(window.modalDialogOpen, true);
    testCase.Window.window.requestActivate();
    tryCompare(window, "active", false);
    compare(window.visible, true);
    mouseClick(findChild(confirmation.contentItem, "confirmSettingsImportAction"));
    tryCompare(confirmation, "visible", false);
    compare(settingsTransferModel.failed, false);
    compare(window.modalDialogOpen, false);
    window.requestActivate();
    tryCompare(window, "active", true);
    testCase.Window.window.requestActivate();
    tryCompare(window, "visible", false);
    verify(window.activationChanges > 0);
  }
}
