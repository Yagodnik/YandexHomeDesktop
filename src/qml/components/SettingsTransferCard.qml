import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  property bool available: false
  property string statusText
  property bool failed: false
  signal exportRequested()
  signal importRequested()
  implicitHeight: content.implicitHeight

  UI.CardSurface { anchors.fill: parent }
  Column {
    id: content
    width: parent.width

    UI.SettingActionRow {
      objectName: "exportSettingsRow"
      width: parent.width
      title: qsTr("Экспорт настроек")
      description: qsTr("Сохранить настройки в JSON-файл.")
      buttonText: qsTr("Сохранить")
      enabled: root.available
      onTriggered: root.exportRequested()
    }
    UI.SettingActionRow {
      objectName: "importSettingsRow"
      width: parent.width
      title: qsTr("Импорт настроек")
      description: qsTr("Заменить настройки данными из файла.")
      buttonText: qsTr("Загрузить")
      enabled: root.available
      onTriggered: root.importRequested()
    }
    UI.SubheadingText {
      x: 10
      width: parent.width - 20
      visible: !root.available
      text: qsTr("Дождитесь загрузки аккаунта.")
      wrapMode: Text.WordWrap
    }
    UI.DefaultText {
      objectName: "settingsTransferStatus"
      x: 10
      width: parent.width - 20
      visible: text.length > 0
      text: root.statusText
      wrapMode: Text.WordWrap
      font.pixelSize: 12
      Accessible.role: root.failed ? Accessible.AlertMessage : Accessible.StaticText
    }
    Item { width: 1; height: 10; visible: !root.available || root.statusText.length > 0 }
  }
}
