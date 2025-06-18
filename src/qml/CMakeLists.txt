qt_add_qml_module(YandexHomeDesktopUi
        URI YandexHomeDesktop.Ui
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/YandexHomeDesktop/Ui"
        VERSION 1.0
        QML_FILES
        ui/HeadingText.qml
        ui/DefaultText.qml
        ui/SubheadingText.qml
        ui/SelectDialog.qml
        ui/MyButton.qml
        ui/MyProgressIndicator.qml
        ui/MySwitch.qml
        ui/AnimatedText.qml
        ui/MyTab.qml
        ui/MyTabs.qml
        ui/MyComboBox.qml
)

qt_add_qml_module(YandexHomeDesktopPages
        URI YandexHomeDesktop.Pages
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/YandexHomeDesktop/Pages"
        VERSION 1.0
        QML_FILES
        pages/DevicesPage.qml
        pages/ScenariosPage.qml
        pages/SettingsPage.qml
        pages/AuthPage.qml
        pages/ErrorPage.qml
        pages/AuthCanceledPage.qml
        pages/LoadingPage.qml
        pages/MainPage.qml
)

qt_add_qml_module(YandexHomeDesktopComponents
        URI YandexHomeDesktop.Components
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/YandexHomeDesktop/Components"
        VERSION 1.0
        QML_FILES
        components/TopBar.qml
        components/ScenarioDelegate.qml
        components/ScenariosList.qml
        components/AccountDetails.qml
)

qt_add_qml_module(appYandexHomeDesktop
        URI YandexHomeDesktop
        VERSION 1.0
        QML_FILES
        Main.qml
)
