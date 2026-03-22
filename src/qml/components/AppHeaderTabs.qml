import QtQuick
import YandexHomeDesktop.Ui as UI

Row {
  id: tabs
  spacing: 16

  property int activeTabIndex

  UI.MyTab {
    text: "Устройства"
    leftCorner: true

    active: tabs.activeTabIndex === 0
    onClicked: tabs.activeTabIndex = 0
  }

  UI.MyTab {
    text: "Сценарии"
    leftCorner: true

    active: tabs.activeTabIndex === 1
    onClicked: tabs.activeTabIndex = 1
  }

  UI.MyTab {
    text: "Настройки"
    leftCorner: true

    active: tabs.activeTabIndex === 2
    onClicked: tabs.activeTabIndex = 2
  }
}