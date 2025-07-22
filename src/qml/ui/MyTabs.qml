import QtQuick
import QtQuick.Controls

Item {
  id: tabView

  property int activeTab: 0
  property ListModel tabs: ListModel {}
  property var pages: []

  function addTab(title, pageComponent) {
    tabs.append({ title: title })
    pages.push(pageComponent)
    if (tabs.count === 1) {
      activeTab = 0
    }
  }

  Row {
    id: tabBar

    Repeater {
      model: tabView.tabs

      delegate: MyTab {
        text: model.title
        active: tabView.activeTab === index
        onClicked: tabView.activeTab = index
      }
    }
  }

  Loader {
    id: pageLoader
    anchors.top: tabBar.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    sourceComponent: tabView.pages[tabView.activeTab]
  }
}
