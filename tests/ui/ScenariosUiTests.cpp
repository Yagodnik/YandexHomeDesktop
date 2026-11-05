#include <QSignalSpy>
#include <QTest>
#include <memory>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlPropertyMap>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include "models/KeybindSettingsModel.h"
#include "../support/ScenariosTestFixtures.h"

using namespace scenarios_test;

class ScenariosUiTests final : public QObject {
  Q_OBJECT
private slots:
  void initTestCase() {
    QQuickStyle::setStyle("Basic");
    qRegisterMetaType<QList<ScenarioObject>>();
    qmlRegisterUncreatableType<KeybindSettingsModel>("YandexHomeDesktop.ViewModels", 1, 0, "KeybindSettingsModel", "Supplied by the application");
    qmlRegisterUncreatableType<ScenariosViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "ScenariosViewModel", "Supplied by the application");
    qmlRegisterUncreatableType<ScenariosModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "ScenariosModel", "Supplied by the view model");
  }

  void QmlPageBindsStateAndForwardsCommands() {
    FakeHomeApi api;
    ScenarioService service(&api);
    ScenariosViewModel view_model(&service);
    QQmlPropertyMap themes;
    for (const auto* key : {"mainText", "controlText", "controlText2", "inactive", "trackColor",
                            "accent", "accent2", "headerBackground", "shadowColor", "background"}) {
      themes.insert(key, QColor("#777777"));
    }
    QQmlEngine engine;
    engine.addImportPath(SCENARIOS_IMPORT_PATH);
    engine.rootContext()->setContextProperty("themes", &themes);
    QSignalSpy warnings(&engine, &QQmlEngine::warnings);
    QQmlComponent component(&engine);
    component.loadFromModule("YandexHomeDesktop.Pages", "ScenariosPage");
    QTRY_VERIFY_WITH_TIMEOUT(component.status() != QQmlComponent::Loading, 5000);
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    std::unique_ptr<QObject> page(component.createWithInitialProperties({
      {"viewModel", QVariant::fromValue(&view_model)},
      {"keybindSettings", QVariant::fromValue(static_cast<KeybindSettingsModel*>(nullptr))}
    }));
    QVERIFY2(page, qPrintable(component.errorString()));
    auto* item = qobject_cast<QQuickItem*>(page.get());
    QVERIFY(item);
    QQuickWindow window;
    window.resize(350, 400);
    item->setParentItem(window.contentItem());
    item->setWidth(350);
    item->setHeight(400);
    window.show();
    auto* stack = page->findChild<QObject*>("scenariosStack");
    auto* refresh = page->findChild<QObject*>("scenarioRefreshButton");
    QVERIFY(stack);
    QVERIFY(refresh);
    QCOMPARE(api.loads.size(), 1);
    QCOMPARE(stack->property("currentIndex").toInt(), 0);
    QVERIFY(!refresh->property("enabled").toBool());

    api.loads.last().Send(std::unexpected(ApiError{ApiErrorKind::Network, "network failure"}));
    QCOMPARE(stack->property("currentIndex").toInt(), 1);
    QVERIFY(refresh->property("enabled").toBool());
    QVERIFY(QMetaObject::invokeMethod(refresh, "refreshClicked"));
    QCOMPARE(api.loads.size(), 2);
    QCOMPARE(stack->property("currentIndex").toInt(), 0);
    api.loads.last().Send(QList<ScenarioObject>{Scenario("a")});
    QCOMPARE(stack->property("currentIndex").toInt(), 2);
    auto* list = page->findChild<QQuickItem*>("scenariosList");
    QVERIFY(list);
    QQuickItem* delegate = nullptr;
    QTRY_VERIFY_WITH_TIMEOUT((delegate = FindDelegate(list)) != nullptr, 5000);
    QCOMPARE(delegate->property("scenario_id").toString(), QString("a"));
    auto* start = delegate->findChild<QQuickItem*>("scenarioStart");
    QVERIFY(start);
    QTRY_COMPARE(delegate->opacity(), 1.0);
    const auto start_position = start->mapToScene(QPointF(start->width() / 2, start->height() / 2)).toPoint();
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, start_position);
    QCOMPARE(api.executions.size(), 1);
    QTRY_VERIFY(delegate->property("is_waiting_response").toBool());
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, start_position);
    QCOMPARE(api.executions.size(), 1);
    api.executions.last().Send(std::unexpected(ApiError{ApiErrorKind::Service, "backend failure"}));
    QTRY_VERIFY(!delegate->property("is_waiting_response").toBool());
    auto* dialog = page->findChild<QObject*>("scenarioErrorDialog");
    QVERIFY(dialog);
    QVERIFY(dialog->property("visible").toBool());
    QCOMPARE(warnings.count(), 0);
  }

private:
  static QQuickItem* FindDelegate(QQuickItem* item) {
    if (item->objectName() == "scenarioDelegate_a") {
      return item;
    }
    for (auto* child : item->childItems()) {
      if (auto* delegate = FindDelegate(child)) {
        return delegate;
      }
    }
    return nullptr;
  }
};

QTEST_MAIN(ScenariosUiTests)
#include "ScenariosUiTests.moc"
