import QtQuick
import QtQuick.Dialogs
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.ViewModels

Item {
  id: root
  property bool openingDialog: false
  readonly property bool dialogOpen: openingDialog || saveDialog.visible
  implicitHeight: snapshot.status.length > 0 ? status.implicitHeight + 16 : 0

  function capture(videoSink) {
    if (!snapshot.Capture(videoSink)) return;
    openingDialog = true;
    saveDialog.selectedFile = snapshot.suggestedFile;
    saveDialog.open();
    openingDialog = false;
  }
  function clear() {
    saveDialog.close();
    snapshot.Clear();
  }

  CameraSnapshotModel { id: snapshot }
  FileDialog {
    id: saveDialog
    objectName: "cameraSnapshotSaveDialog"
    title: qsTr("Сохранить снимок камеры")
    fileMode: FileDialog.SaveFile
    defaultSuffix: "png"
    nameFilters: [qsTr("Изображение PNG (*.png)")]
    onAccepted: snapshot.Save(selectedFile)
    onRejected: snapshot.Clear()
  }
  UI.DefaultText {
    id: status
    objectName: "cameraSnapshotStatus"
    anchors.centerIn: parent
    width: parent.width - 32
    font.pixelSize: 12
    horizontalAlignment: Text.AlignHCenter
    wrapMode: Text.Wrap
    text: snapshot.status
  }
  Timer {
    interval: 5000
    running: snapshot.status.length > 0 && !root.dialogOpen
    onTriggered: snapshot.Clear()
  }
}
