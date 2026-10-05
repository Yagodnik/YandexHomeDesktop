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
        ui/Skeleton.qml
        ui/CardSurface.qml
        ui/RefreshHeader.qml
        ui/LoadingPane.qml
        ui/LoadErrorPane.qml
        ui/PageSurface.qml
        ui/PageStates.qml
        ui/InsetPane.qml
        ui/ScrollColumn.qml
        ui/ListScrollBar.qml
        ui/RetryPane.qml
        ui/MessageActionsPane.qml
        ui/LinkLabel.qml
        ui/LinkFooter.qml
        ui/SettingSwitchRow.qml
        ui/SettingChoiceRow.qml
        ui/SettingLinkRow.qml
        ui/Shadow.qml
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
        pages/DevicePage.qml
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
        components/DeviceHeader.qml
        components/DeviceAttributeSection.qml
        components/DeviceControlsPane.qml
        components/PropertyValueCard.qml
        components/SignInCard.qml
        components/HouseholdDelegate.qml
        components/HouseholdPicker.qml
        components/BasicSettingsCard.qml
        components/RoomsPane.qml
        components/ScenariosPane.qml
)
