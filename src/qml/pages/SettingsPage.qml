import QtQuick
import YandexHomeDesktop.ViewModels
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

UI.PageSurface {
  id: root
  required property RestViewModel restModel
  required property SettingsTransferModel transferModel
  required property ShortcutViewModel shortcutModel
  property var mcpModel: null
  modalDialogOpen: transferDialogs.dialogOpen
  Components.SettingsTransferDialogs { id: transferDialogs; viewModel: root.transferModel }
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
      notchAvailable: platformService.notchSupported
      notchEnabled: settings.notchEnabled
      notchTitle: qsTr("Открывать у выреза экрана")
      notchDescription: qsTr("Показывать приложение при наведении\nна вырез экрана")
      onNotchToggled: function(enabled) { settings.notchEnabled = enabled; }
      themeTitle: qsTr("Тема")
      themeNames: [qsTr("Светлая"), qsTr("Тёмная")]
      currentTheme: settings.currentTheme
      keybindsTitle: qsTr("Сочетания клавиш")
      keybindsDescription: qsTr("Показывать кнопки назначения\nрядом с умениями устройств")
      showKeybinds: settings.showKeybinds
      projectLinkText: qsTr("GitHub")

      onTrayModeToggled: function(enabled) { settings.trayModeEnabled = enabled; }

      onThemeSelected: function(index) {
        settings.currentTheme = index;
        themes.SetTheme(index);
      }

      onKeybindsToggled: function(visible) { settings.showKeybinds = visible; }

      onProjectLinkClicked: Qt.openUrlExternally("https://github.com/Yagodnik/YandexHomeDesktop")
    }

    UI.HeadingText {
      objectName: "settingsTransferHeading"
      text: qsTr("Перенос настроек")
      width: parent.width
    }

    Components.SettingsTransferCard {
      objectName: "settingsTransferCard"
      width: parent.width
      available: root.transferModel.available
      statusText: root.transferModel.status
      failed: root.transferModel.failed
      onExportRequested: transferDialogs.openExport()
      onImportRequested: transferDialogs.openImport()
    }

    UI.HeadingText {
      text: qsTr("Дополнительно")
      width: parent.width
    }

    Components.ShortcutSettingsCard {
      objectName: "shortcutSettings"
      width: parent.width
      shortcutsEnabled: root.shortcutModel.enabled
      backgroundEnabled: root.shortcutModel.background
      busy: root.shortcutModel.busy
      statusText: root.shortcutModel.status
      errorText: root.shortcutModel.error
      onEnabledToggled: function(enabled) { root.shortcutModel.SetEnabled(enabled); }
      onBackgroundToggled: function(enabled) { root.shortcutModel.SetBackground(enabled); }
    }

    Components.AdvancedSettingsCard {
      objectName: "advancedSettings"
      width: parent.width
      title: qsTr("REST API")
      description: qsTr("Локальный доступ для скриптов")
      restEnabled: root.restModel.enabled
      port: root.restModel.port
      portEditable: !root.restModel.running
      busy: root.restModel.busy
      statusText: root.restModel.busy ? qsTr("Подождите…")
        : root.restModel.running ? root.restModel.url
        : qsTr("Сервер остановлен")
      errorText: root.restModel.error
      onRestToggled: function(enabled) { root.restModel.SetEnabled(enabled); }
      onPortChangedByUser: function(port) { root.restModel.SetPort(port); }
    }

    Components.AdvancedSettingsCard {
      objectName: "mcpSettings"
      width: parent.width
      visible: root.mcpModel !== null && root.mcpModel !== undefined
      height: visible ? implicitHeight : 0
      title: qsTr("MCP-сервер")
      description: qsTr("Доступ для AI-агентов")
      restEnabled: visible ? root.mcpModel.running : false
      port: visible ? root.mcpModel.port : 8767
      busy: visible ? root.mcpModel.busy : false
      statusText: !visible ? "" : root.mcpModel.busy ? qsTr("Подождите…")
        : root.mcpModel.running ? root.mcpModel.url : qsTr("Сервер остановлен")
      errorText: visible ? root.mcpModel.error : ""
      onRestToggled: function(enabled) { root.mcpModel.setEnabled(enabled); }
      onPortChangedByUser: function(port) { root.mcpModel.setPort(port); }
    }
  }
}
