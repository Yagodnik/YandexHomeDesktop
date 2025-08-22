#qt_add_qml_module(YandexHomeDesktopUi
#        URI YandexHomeDesktop.Ui
#        OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/YandexHomeDesktop/Ui"
#        VERSION 1.0
#        QML_FILES
#        YandexHomeDesktop/ui/HeadingText.qml
#        YandexHomeDesktop/ui/DefaultText.qml
#        YandexHomeDesktop/ui/SubheadingText.qml
#        YandexHomeDesktop/ui/SelectDialog.qml
#        YandexHomeDesktop/ui/MyButton.qml
#        YandexHomeDesktop/ui/MyProgressIndicator.qml
#        YandexHomeDesktop/ui/MySwitch.qml
#        YandexHomeDesktop/ui/AnimatedText.qml
#        YandexHomeDesktop/ui/MyTab.qml
#        YandexHomeDesktop/ui/MyTabs.qml
#        YandexHomeDesktop/ui/MyComboBox.qml
#        YandexHomeDesktop/ui/MySlider.qml
#        YandexHomeDesktop/ui/ImageButton.qml
#        YandexHomeDesktop/ui/ErrorDialog.qml
#        YandexHomeDesktop/ui/Skeleton.qml
#)
#
#qt_add_qml_module(YandexHomeDesktopPages
#        URI YandexHomeDesktop.Pages
#        OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/YandexHomeDesktop/Pages"
#        VERSION 1.0
#        QML_FILES
#        YandexHomeDesktop/pages/DevicesPage.qml
#        YandexHomeDesktop/pages/ScenariosPage.qml
#        YandexHomeDesktop/pages/SettingsPage.qml
#        YandexHomeDesktop/pages/AuthPage.qml
#        YandexHomeDesktop/pages/ErrorPage.qml
#        YandexHomeDesktop/pages/AuthCanceledPage.qml
#        YandexHomeDesktop/pages/LoadingPage.qml
#        YandexHomeDesktop/pages/MainPage.qml
#)
#
#qt_add_qml_module(YandexHomeDesktopComponents
#        URI YandexHomeDesktop.Components
#        OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/YandexHomeDesktop/Components"
#        VERSION 1.0
#        QML_FILES
#        YandexHomeDesktop/components/TopBar.qml
#        YandexHomeDesktop/components/ScenarioDelegate.qml
#        YandexHomeDesktop/components/ScenariosList.qml
#        YandexHomeDesktop/components/AccountDetails.qml
#        YandexHomeDesktop/components/DeviceDelegate.qml
#        YandexHomeDesktop/components/RoomDevicesList.qml
#        YandexHomeDesktop/components/AdaptiveGrid.qml
#)

add_subdirectory(YandexHomeDesktop/Ui)
#add_subdirectory(YandexHomeDesktop/Pages)
add_subdirectory(YandexHomeDesktop/Components)
