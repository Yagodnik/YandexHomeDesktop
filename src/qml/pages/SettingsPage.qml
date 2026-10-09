import QtQuick
import YandexHomeDesktop.ViewModels
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

UI.PageSurface {
  id: root
  required property RestViewModel restModel
  UI.ScrollColumn {
    objectName: "settingsContent"
    anchors.fill: parent
    spacing: 12

    UI.HeadingText {
      text: qsTr("Настройки")
      width: parent.width
    }

    Components.AccountDetails {}

    Components.BasicSettingsCard {
      objectName: "basicSettings"
      width: parent.width
      trayTitle: qsTr("Tray-режим")
      trayDescription: qsTr("Приложение будет отображаться\nкак иконка на панели задач")
      trayModeEnabled: settings.trayModeEnabled
      themeTitle: qsTr("Тема")
      themeNames: [qsTr("Светлая"), qsTr("Тёмная")]
      currentTheme: settings.currentTheme
      projectLinkText: qsTr("GitHub")

      onTrayModeToggled: function(enabled) {
        settings.trayModeEnabled = enabled;
        if (enabled) {
          platformService.ShowOnlyInTray();
        } else {
          platformService.ShowAsApp();
        }
      }

      onThemeSelected: function(index) {
        settings.currentTheme = index;
        themes.SetTheme(index);
      }

      onProjectLinkClicked: Qt.openUrlExternally("https://github.com/Yagodnik/YandexHomeDesktop")
    }

    UI.HeadingText {
      text: qsTr("Дополнительно")
      width: parent.width
    }

    Components.AdvancedSettingsCard {
      objectName: "advancedSettings"
      width: parent.width
      title: qsTr("REST API")
      description: qsTr("Локальный доступ для скриптов")
      restEnabled: root.restModel.enabled
      busy: root.restModel.busy
      statusText: root.restModel.busy ? qsTr("Подождите…")
        : root.restModel.running ? root.restModel.url
        : qsTr("Сервер остановлен")
      errorText: root.restModel.error
      onRestToggled: function(enabled) { root.restModel.SetEnabled(enabled); }
    }
  }
}
