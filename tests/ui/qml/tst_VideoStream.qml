import QtQuick
import QtQuick.Window
import QtQuick.Dialogs
import QtQuick.Controls
import QtTest
import YandexHomeDesktop.Pages as Pages
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.ViewModels

TestCase {
  id: testCase
  name: "VideoStream"
  width: 350
  height: 460
  visible: true
  when: windowShown

  Component { id: pageComponent; Pages.DevicePage { width: 350; height: 460 } }
  Component { id: snapshotModelComponent; CameraSnapshotModel {} }
  Component {
    id: trayComponent
    UI.TrayWindow {
      width: 350
      height: 460
      visible: true
      trayModeEnabled: true
      modalDialogOpen: cameraPage.modalDialogOpen
      Pages.DevicePage { id: cameraPage; objectName: "trayCameraPage"; anchors.fill: parent }
    }
  }

  function startVideo(page) {
    const control = findChild(page, "videoStreamControl");
    waitForRendering(page);
    mouseClick(findChild(control, "videoStreamPlayAction"));
    deviceTestApi.ReplyVideoStream(demoVideoSource, true);
    tryVerify(function() { return findChild(control, "videoStreamPlayer") !== null; });
    const player = findChild(control, "videoStreamPlayer");
    tryCompare(player, "receivedFrame", true, 10000);
    tryCompare(findChild(control, "videoStreamSnapshotAction"), "enabled", true, 10000);
    waitForRendering(page);
    return control;
  }

  function init() {
    // CTest's software renderer converts snapshots on the CPU and reports this once.
    failOnWarning(/^(?!.*No RHI backend\. Using CPU conversion\.$).*/);
    deviceViewModel.ForgetDevice();
    deviceTestApi.reset();
    deviceTestApi.SetCapability("video", {}, { protocols: ["hls"] });
    deviceViewModel.LoadDevice("camera");
    deviceTestApi.ReplyDevice(true);
    deviceViewModel.StopPolling();
    themes.SetTheme(0);
  }

  function cleanup() {
    deviceViewModel.ForgetDevice();
    testCase.Window.window.width = 600;
    testCase.Window.window.height = 600;
  }

  function test_playRetryAndStop() {
    const page = createTemporaryObject(pageComponent, testCase);
    verify(page !== null);
    const control = findChild(page, "videoStreamControl");
    verify(control !== null);
    const play = findChild(control, "videoStreamPlayAction");
    const stop = findChild(control, "videoStreamStopAction");
    const snapshot = findChild(control, "videoStreamSnapshotAction");
    compare(play.display, AbstractButton.IconOnly);
    compare(stop.display, AbstractButton.IconOnly);
    compare(snapshot.display, AbstractButton.IconOnly);
    compare(snapshot.enabled, false);
    waitForRendering(page);
    if (settingsPreviewPath.length > 0) grabImage(page).save(settingsPreviewPath + "-video-idle.png");
    mouseClick(play);
    compare(deviceTestApi.streamRequestCount, 1);
    compare(deviceTestApi.streamDeviceId, "camera");
    compare(deviceTestApi.actionCount, 0);
    compare(play.enabled, false);
    deviceTestApi.ReplyVideoStream("", false);
    compare(control.streamFailed, true);
    compare(play.enabled, true);
    compare(findChild(page, "deviceErrorDialog").visible, false);
    waitForRendering(page);
    if (settingsPreviewPath.length > 0) grabImage(page).save(settingsPreviewPath + "-video-retry.png");
    mouseClick(play);
    deviceTestApi.ReplyVideoStream(demoVideoSource, true);
    tryVerify(function() { return findChild(control, "videoStreamPlayer") !== null; });
    const player = findChild(control, "videoStreamPlayer");
    tryCompare(player, "playing", true, 10000);
    tryCompare(player, "frameWidth", 320, 10000);
    tryCompare(player, "receivedFrame", true, 10000);
    waitForRendering(player);
    compare(play.visible, false);
    if (settingsPreviewPath.length > 0) grabImage(page).save(settingsPreviewPath + "-video.png");
    mouseClick(stop);
    compare(deviceViewModel.videoStreamUrl.toString(), "");
    tryVerify(function() { return findChild(control, "videoStreamPlayer") === null; });
    compare(play.visible, true);
    compare(snapshot.enabled, false);
  }

  function test_snapshotSavesFrame_data() {
    return [{ tag: "light", theme: 0, width: 320 }, { tag: "dark", theme: 1, width: 350 }];
  }

  function test_snapshotSavesFrame(data) {
    testCase.Window.window.width = data.width;
    testCase.Window.window.height = 460;
    themes.SetTheme(data.theme);
    const window = createTemporaryObject(trayComponent, null, { width: data.width });
    verify(window !== null);
    const page = findChild(window.contentItem, "trayCameraPage");
    const control = startVideo(page);
    const viewport = findChild(control, "videoStreamViewport");
    const snapshot = findChild(control, "videoStreamSnapshotAction");
    const position = snapshot.mapToItem(viewport, 0, 0);
    verify(position.x >= 0 && position.x + snapshot.width <= viewport.width);
    verify(position.y >= 0 && position.y + snapshot.height <= viewport.height);
    if (settingsPreviewPath.length > 0) grabImage(page).save(settingsPreviewPath + "-video-" + data.tag + ".png");
    const dialog = findChild(page, "cameraSnapshotSaveDialog");
    dialog.options = FileDialog.DontUseNativeDialog;
    snapshot.forceActiveFocus();
    keyClick(Qt.Key_Space);
    tryCompare(dialog, "visible", true);
    compare(window.modalDialogOpen, true);
    verify(dialog.selectedFile.toString().endsWith(".png"));
    testCase.Window.window.requestActivate();
    tryCompare(window, "active", false);
    compare(window.visible, true);
    // The copied frame must survive destruction of the video player while choosing a path.
    deviceViewModel.StopVideoStream();
    const destination = cameraSnapshotTestFile + "-" + data.tag + ".png";
    dialog.selectedFile = destination;
    dialog.done(FileDialog.Accepted);
    tryCompare(window, "modalDialogOpen", false);
    const saved = uiTestHelper.snapshotInfo(destination);
    compare(saved.width, 320);
    compare(saved.height, 180);
    verify(saved.varied, "The PNG must contain the decoded camera image, without the controls");
    verify(findChild(page, "cameraSnapshotStatus").text.length > 0);
    compare(deviceTestApi.actionCount, 0);
  }

  function test_snapshotCancelAndFailure() {
    const model = createTemporaryObject(snapshotModelComponent, testCase);
    compare(model.Capture(null), false);
    verify(model.status.length > 0);
    compare(model.Save(cameraSnapshotTestFile), false);
    const saveFailure = model.status;
    model.Clear();
    compare(model.status, "");
    const page = createTemporaryObject(pageComponent, testCase);
    const control = startVideo(page);
    const snapshot = findChild(control, "videoStreamSnapshotAction");
    const dialog = findChild(page, "cameraSnapshotSaveDialog");
    dialog.options = FileDialog.DontUseNativeDialog;
    mouseClick(snapshot);
    tryCompare(dialog, "visible", true);
    dialog.reject();
    tryCompare(page, "modalDialogOpen", false);
    compare(findChild(page, "cameraSnapshotStatus").text, "");
    compare(findChild(control, "videoStreamPlayer").playing, true);
    mouseClick(snapshot);
    tryCompare(dialog, "visible", true);
    dialog.selectedFile = cameraSnapshotTestFile + "/missing/snapshot.png";
    dialog.done(FileDialog.Accepted);
    compare(findChild(page, "cameraSnapshotStatus").text, saveFailure);
    // Leaving the device cancels an outstanding save dialog and discards the frame.
    mouseClick(snapshot);
    tryCompare(dialog, "visible", true);
    deviceViewModel.ForgetDevice();
    tryCompare(dialog, "visible", false);
    compare(findChild(page, "cameraSnapshotStatus").text, "");
  }

  function test_cancelAndHide() {
    const page = createTemporaryObject(pageComponent, testCase);
    const control = findChild(page, "videoStreamControl");
    const play = findChild(control, "videoStreamPlayAction");
    const stop = findChild(control, "videoStreamStopAction");
    waitForRendering(page);
    mouseClick(play);
    compare(deviceTestApi.streamRequestCount, 1);
    waitForRendering(page);
    mouseClick(stop);
    compare(deviceViewModel.videoStreamLoading, false);
    deviceTestApi.ReplyVideoStream(demoVideoSource, true);
    compare(deviceViewModel.videoStreamUrl.toString(), "");
    waitForRendering(page);
    mouseClick(play);
    compare(deviceTestApi.streamRequestCount, 2);
    page.visible = false;
    compare(deviceViewModel.videoStreamLoading, false);
    deviceTestApi.ReplyVideoStream(demoVideoSource, true);
    compare(deviceViewModel.videoStreamUrl.toString(), "");
  }
}
