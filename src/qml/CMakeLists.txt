qt_add_qml_module(YandexHomeDesktopUi
        URI YandexHomeDesktop.Ui
        OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/YandexHomeDesktop/Ui"
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
        ui/MySlider.qml
        ui/ImageButton.qml
        ui/ErrorDialog.qml
)

qt_add_qml_module(YandexHomeDesktopPages
        URI YandexHomeDesktop.Pages
        OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/YandexHomeDesktop/Pages"
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
        OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/YandexHomeDesktop/Components"
        VERSION 1.0
        QML_FILES
        components/TopBar.qml
        components/ScenarioDelegate.qml
        components/ScenariosList.qml
        components/AccountDetails.qml
        components/DeviceDelegate.qml
        components/RoomDevicesList.qml
        components/AdaptiveGrid.qml
)


qt_add_qml_module(YandexHomeDesktopIotControls
        URI YandexHomeDesktop.IotControls
        OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/YandexHomeDesktop/IotControls"
        VERSION 1.0
        QML_FILES
        controls/OnOff.qml
        controls/Range.qml
        controls/Mode.qml
        controls/Unsupported.qml
)
