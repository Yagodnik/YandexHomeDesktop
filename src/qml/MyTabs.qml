import QtQuick

Row {
  id: tabBar

  property var tabs: []
  property var defaultActiveTab: 0

  property var activeTab: defaultActiveTab

  Repeater {
    model: tabBar.tabs.length

    delegate: MyTab {
      text: tabBar.tabs[index].title
      active: tabBar.activeTab === index

      onClicked: {
        tabBar.activeTab = index
      }
    }
  }
}
