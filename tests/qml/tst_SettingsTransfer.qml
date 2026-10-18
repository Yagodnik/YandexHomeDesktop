import QtQuick
import QtQuick.Window
import QtQuick.Dialogs
import QtTest
import YandexHomeDesktop.Pages as Pages

TestCase {
  id: testCase
  name: "SettingsTransfer"
  width: 350
  height: 460
  visible: true
  when: windowShown
  Component {
    id: settingsPage
    Pages.SettingsPage { height: 460; restModel: restViewModel; transferModel: settingsTransferModel }
  }
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
}
