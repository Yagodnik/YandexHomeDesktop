# Included from the project root to retain executable and QML import paths.
target_compile_definitions(ApiFixtureTests PRIVATE "$<$<CONFIG:Debug>:YH_DEBUG_FAKE_API>")
target_link_libraries(ApiFixtureTests PRIVATE "$<$<CONFIG:Debug>:YandexAuth>")
target_sources(HomeTests PRIVATE tests/QmlTestEnvironment.cpp tests/QmlTestEnvironment.h)
target_compile_definitions(HomeTests PRIVATE
  HOME_QML_TESTS HOME_IMPORT_PATH="${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(HomeTests PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::QuickTest Iot YandexApiAdapter)
add_dependencies(HomeTests YandexHomeDesktopComponentsplugin
  YandexHomeDesktopUiplugin YandexHomeDesktopPagesplugin)
target_compile_definitions(ScenariosTests PRIVATE
  SCENARIOS_QML_TESTS SCENARIOS_IMPORT_PATH="${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ScenariosTests PRIVATE Qt6::Quick Qt6::QuickControls2)
add_dependencies(ScenariosTests YandexHomeDesktopComponentsplugin
  YandexHomeDesktopUiplugin YandexHomeDesktopPagesplugin)
add_executable(QmlPageLoadTest tests/QmlPageLoadTest.cpp
  tests/QmlTestEnvironment.cpp tests/QmlTestEnvironment.h)
target_link_libraries(QmlPageLoadTest PRIVATE Qt6::Gui Qt6::Qml Qt6::Test Models Utils Iot YandexApiAdapter)
add_dependencies(QmlPageLoadTest YandexHomeDesktopComponentsplugin
  YandexHomeDesktopUiplugin YandexHomeDesktopPagesplugin)
foreach(_qml_page AuthPage AuthCanceledPage ErrorPage LoadingPage MainPage
    DevicePage DevicesPage ScenariosPage SettingsPage)
  yh_add_test(NAME ${_qml_page}Loads COMMAND QmlPageLoadTest
    "${CMAKE_CURRENT_SOURCE_DIR}/src/qml/pages/${_qml_page}.qml"
    "${CMAKE_CURRENT_BINARY_DIR}")
  set_tests_properties(${_qml_page}Loads PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_QUICK_CONTROLS_STYLE=Basic;QT_QPA_FONTDIR=${PROJECT_SOURCE_DIR}/resources/fonts")
endforeach()

add_executable(QmlLayoutTests tests/QmlLayoutTests.cpp
  tests/QmlTestEnvironment.cpp tests/QmlTestEnvironment.h)
target_link_libraries(QmlLayoutTests PRIVATE Qt6::QuickTest Qt6::Qml Qt6::Gui Models Utils Iot YandexApiAdapter)
add_dependencies(QmlLayoutTests YandexHomeDesktopComponentsplugin
  YandexHomeDesktopUiplugin YandexHomeDesktopPagesplugin)
yh_add_test(NAME QmlLayouts COMMAND QmlLayoutTests
  -input "${CMAKE_CURRENT_SOURCE_DIR}/tests/qml"
  -import "${CMAKE_CURRENT_BINARY_DIR}")
set_tests_properties(QmlLayouts PROPERTIES
  TIMEOUT 90
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_QUICK_CONTROLS_STYLE=Basic;QT_QPA_FONTDIR=${PROJECT_SOURCE_DIR}/resources/fonts")

target_sources(QmlPageLoadTest PRIVATE ${DESKTOP_RESOURCES})
target_sources(QmlLayoutTests PRIVATE ${DESKTOP_RESOURCES} ${DEBUG_FIXTURE_RESOURCES})
target_sources(ScenariosTests PRIVATE ${DESKTOP_RESOURCES})
target_sources(HomeTests PRIVATE ${DESKTOP_RESOURCES})
