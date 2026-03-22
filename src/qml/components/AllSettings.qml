import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: basicSettings
  width: parent.width
  height: elements.implicitHeight

  Rectangle {
    id: background2
    anchors.fill: parent
    color: themes.headerBackground
    radius: 16
  }

  Column {
    id: elements
    anchors.fill: parent

    Components.SettingsOption {
      optionName: "Tray-режим"
      optionDescription: "Приложение будет отображаться\nкак иконка на панели задач"

      onToggled: (checked) => {
        settings.trayModeEnabled = checked

        if (checked) { platformService.ShowOnlyInTray(); }
        else { platformService.ShowAsApp(); }
      }
    }

    Components.SettingsComboOption {
      optionName: "Тема"

      currentIndex: settings.currentTheme

      model: ListModel {
        ListElement {
          displayText: "Светлая"
        }

        ListElement {
          displayText: "Тёмная"
        }
      }

      onClicked: (index, currentText) => {
        console.log("Selected index:", index, "value:", currentText)

        settings.currentTheme = index;
        themes.SetTheme(currentIndex);
      }
    }

    Components.SettingsReference {
      optionName: "GitHub"
      optionUrl: "https://github.com/Yagodnik/YandexHomeDesktop"
    }
  }
}