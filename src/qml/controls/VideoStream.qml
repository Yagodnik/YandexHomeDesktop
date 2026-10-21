import QtQuick
import QtQuick.Window
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

UI.CardSurface {
  id: root
  objectName: "videoStreamControl"
  height: content.implicitHeight + 24

  property url streamSource
  property bool streamLoading: false
  property bool streamFailed: false
  property bool playbackFailed: false
  readonly property bool presentationVisible: visible && (Window.window === null || Window.window.visible)
  signal streamRequested()
  signal streamStopRequested()
  signal snapshotRequested(var videoSink)

  onStreamSourceChanged: playbackFailed = false
  onPresentationVisibleChanged: {
    if (!presentationVisible && (streamLoading || streamSource.toString().length > 0)) streamStopRequested();
  }

  Column {
    id: content
    x: 12
    y: 12
    width: parent.width - 24
    spacing: 12

    UI.SubheadingText { text: qsTr("Видеопоток") }

    Rectangle {
      id: viewport
      objectName: "videoStreamViewport"
      width: parent.width
      height: width * 9 / 16
      radius: 8
      color: "#111111"
      clip: true

      Loader {
        id: playerLoader
        anchors.fill: parent
        active: root.presentationVisible && root.streamSource.toString().length > 0 && !root.playbackFailed
        sourceComponent: Components.VideoStreamPlayer {
          source: root.streamSource
          onPlaybackFailed: root.playbackFailed = true
        }
      }

      UI.DefaultText {
        objectName: "videoStreamStatus"
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -12
        width: parent.width - 24
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        color: "white"
        visible: !playerLoader.item || !playerLoader.item.playing
        text: root.streamFailed || root.playbackFailed ? qsTr("Не удалось открыть видеопоток")
          : root.streamLoading || playerLoader.active ? qsTr("Подключение к камере…")
          : qsTr("Видео с камеры")
      }

      Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 52
        gradient: Gradient {
          GradientStop { position: 0; color: "transparent" }
          GradientStop { position: 1; color: "#d9000000" }
        }

        Row {
          anchors.left: parent.left
          anchors.bottom: parent.bottom
          anchors.margins: 4
          spacing: 2

          UI.IconButton {
            objectName: "videoStreamPlayAction"
            foregroundColor: "white"
            text: root.streamFailed || root.playbackFailed ? qsTr("Повторить") : qsTr("Смотреть")
            icon.source: root.streamFailed || root.playbackFailed ? "qrc:/images/reload.svg" : "qrc:/images/play.svg"
            enabled: !root.streamLoading
            visible: root.streamSource.toString().length === 0 || root.playbackFailed
            onClicked: root.streamRequested()
          }

          UI.IconButton {
            objectName: "videoStreamStopAction"
            foregroundColor: "white"
            text: qsTr("Остановить")
            icon.source: "qrc:/images/video-stop.svg"
            visible: root.streamLoading || root.streamSource.toString().length > 0
            onClicked: root.streamStopRequested()
          }
        }

        UI.IconButton {
          objectName: "videoStreamSnapshotAction"
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          anchors.margins: 4
          foregroundColor: "white"
          text: qsTr("Сделать снимок")
          icon.source: "qrc:/images/video-snapshot.svg"
          enabled: playerLoader.item !== null && playerLoader.item.playing && playerLoader.item.receivedFrame
          onClicked: root.snapshotRequested(playerLoader.item.videoSink)
        }
      }
    }
  }
}
