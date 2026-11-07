import QtQuick
import QtMultimedia

Item {
  id: root
  objectName: "videoStreamPlayer"
  required property url source
  readonly property alias videoSink: output.videoSink
  readonly property bool playing: player.playbackState === MediaPlayer.PlayingState
  readonly property int frameWidth: output.videoSink.videoSize.width
  property bool receivedFrame: false
  signal playbackFailed()

  Connections {
    target: output.videoSink
    function onVideoFrameChanged() {
      if (output.videoSink.videoSize.width > 0) root.receivedFrame = true;
    }
  }

  MediaPlayer {
    id: player
    source: root.source
    autoPlay: true
    loops: MediaPlayer.Infinite
    videoOutput: output
    audioOutput: AudioOutput {}
    onErrorOccurred: root.playbackFailed()
  }

  VideoOutput {
    id: output
    anchors.fill: parent
    fillMode: VideoOutput.PreserveAspectFit
  }
}
