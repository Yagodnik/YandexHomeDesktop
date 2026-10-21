import QtQuick
import QtQuick.Controls

Dialog {
  enter: Transition {
    ParallelAnimation {
      NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
      NumberAnimation { property: "scale"; from: 0.9; to: 1; duration: 200; easing.type: Easing.OutBack }
    }
  }
  exit: Transition {
    ParallelAnimation {
      NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
      NumberAnimation { property: "scale"; from: 1; to: 0.9; duration: 200; easing.type: Easing.InBack }
    }
  }
}
