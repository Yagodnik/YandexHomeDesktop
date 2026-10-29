import QtQuick
import QtQuick.Controls
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: rows.implicitHeight
  property string title
  property string description
  property string statusText
  property string errorText
  property bool restEnabled: false
  property bool busy: false
  property int port: 1
  property bool portEditable: true
  signal restToggled(bool enabled)
  signal portChangedByUser(int port)

  UI.CardSurface { anchors.fill: parent }

  Column {
    id: rows
    width: root.width

    UI.SettingSwitchRow {
      objectName: "restSettingRow"
      width: parent.width
      title: root.title
      description: root.description
      checked: root.restEnabled
      enabled: !root.busy
      onToggled: function(checked) { root.restToggled(checked); }
    }
    Item {
      width: parent.width
      height: 46
      UI.DefaultText {
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        text: qsTr("Порт")
        opacity: portInput.enabled ? 1 : 0.5
      }
      TextField {
        id: portInput
        objectName: "serverPortInput"
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        width: 96
        height: 32
        enabled: root.portEditable && !root.busy && !root.restEnabled
        color: themes.controlText2
        font.pixelSize: 14
        horizontalAlignment: Text.AlignHCenter
        inputMethodHints: Qt.ImhDigitsOnly
        validator: IntValidator { bottom: 1; top: 65535 }
        background: Rectangle { radius: 8; color: themes.accent; opacity: portInput.enabled ? 1 : 0.5 }
        onEditingFinished: {
          if (acceptableInput && Number(text) !== root.port) root.portChangedByUser(Number(text));
          text = String(root.port);
        }
        onActiveFocusChanged: if (!activeFocus && !acceptableInput) text = String(root.port)
        Component.onCompleted: text = String(root.port)
      }
      Connections {
        target: root
        function onPortChanged() { if (!portInput.activeFocus) portInput.text = String(root.port); }
      }
    }
    UI.SubheadingText {
      x: 10
      width: parent.width - 20
      visible: root.restEnabled || !root.portEditable
      text: qsTr("Остановите сервер, чтобы изменить порт.")
      wrapMode: Text.Wrap
    }
    UI.SubheadingText {
      x: 10
      width: parent.width - 20
      text: root.statusText
      visible: text.length > 0
      wrapMode: Text.Wrap
    }
    UI.SubheadingText {
      x: 10
      width: parent.width - 20
      text: root.errorText
      visible: text.length > 0
      wrapMode: Text.Wrap
    }
    Item { width: 1; height: 10; visible: root.statusText.length > 0 || root.errorText.length > 0 }
  }
}
