import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.ViewModels

Item {
  id: root
  required property SettingsTransferModel viewModel
  property bool openingFileDialog: false
  readonly property bool dialogOpen: openingFileDialog || exportDialog.visible || importDialog.visible || confirmation.visible
  function openExport() {
    if (!viewModel.available) return;
    openingFileDialog = true;
    exportDialog.open();
    openingFileDialog = false;
  }
  function openImport() {
    if (!viewModel.available) return;
    openingFileDialog = true;
    importDialog.open();
    openingFileDialog = false;
  }

  FileDialog {
    id: exportDialog
    objectName: "exportSettingsDialog"
    title: qsTr("Сохранить настройки")
    fileMode: FileDialog.SaveFile
    defaultSuffix: "json"
    nameFilters: [qsTr("Настройки JSON (*.json)")]
    onAccepted: root.viewModel.Export(selectedFile)
  }
  FileDialog {
    id: importDialog
    objectName: "importSettingsDialog"
    title: qsTr("Загрузить настройки")
    fileMode: FileDialog.OpenFile
    nameFilters: [qsTr("Настройки JSON (*.json)")]
    onAccepted: root.viewModel.PrepareImport(selectedFile)
  }
  Connections {
    target: root.viewModel
    function onChanged() {
      if (root.viewModel.importPending) confirmation.open();
      else confirmation.close();
      if (!root.viewModel.available) {
        exportDialog.close();
        importDialog.close();
      }
    }
  }
  UI.AnimatedDialog {
    id: confirmation
    objectName: "confirmSettingsImport"
    parent: Overlay.overlay
    modal: true
    focus: true
    width: Math.min(336, parent.width - 24)
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    padding: 20
    onRejected: root.viewModel.CancelImport()
    background: UI.CardSurface {}
    contentItem: Column {
      spacing: 16
      UI.HeadingText {
        width: parent.width
        text: qsTr("Заменить настройки?")
        font.pixelSize: 20
        wrapMode: Text.WordWrap
      }
      UI.DefaultText {
        width: parent.width
        text: qsTr("Тема, tray-режим, сочетания клавиш и настройки избранного и комнат будут заменены данными из файла.")
        wrapMode: Text.WordWrap
        font.pixelSize: 13
      }
      UI.MyButton {
        objectName: "confirmSettingsImportAction"
        width: parent.width
        text: qsTr("Загрузить настройки")
        onClicked: root.viewModel.ConfirmImport()
      }
      UI.MyButton {
        objectName: "cancelSettingsImportAction"
        width: parent.width
        text: qsTr("Отмена")
        onClicked: confirmation.reject()
      }
    }
  }
}
