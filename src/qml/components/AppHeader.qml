import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: appHeader

  width: parent.width
  height: layout.implicitHeight

  Rectangle {
    id: background
    anchors.fill: parent
    color: themes.headerBackground
  }

  property int activeTabIndex: 0
  property bool showHouseholdSelector: true

  Column {
    id: layout
    anchors.fill: parent
    spacing: 12

    Components.HouseholdSelectWidget {
      id: householdSelect
      width: parent.width
      anchors.left: parent.left
      anchors.leftMargin: 16

      visible: showHouseholdSelector

      currentHouseholdName: householdsModel.currentHouseholdName

      onClicked: () => householdSelectDialog.open();
    }

    Components.AppHeaderTabs {
      id: tabs

      anchors.left: parent.left
      anchors.leftMargin: 16
      width: parent.width

      activeTabIndex: appHeader.activeTabIndex
      onActiveTabIndexChanged: appHeader.activeTabIndex = tabs.activeTabIndex
    }
  }
}
