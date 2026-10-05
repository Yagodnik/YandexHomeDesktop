#include <QAbstractItemModelTester>
#ifdef SCENARIOS_QML_TESTS
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlPropertyMap>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#endif
#include <QPointer>
#include <QSignalSpy>
#include <QTest>

#include "api/IHomeApi.h"
#include "models/ScenariosModel/ScenariosViewModel.h"

template<typename T>
struct PendingResult {
  QPointer<QObject> context;
  ApiResultHandler<T> handler;
  QString scenario_id;

  void Send(ApiResult<T> result) const {
    if (context) {
      handler(std::move(result));
    }
  }
};

class FakeHomeApi : public IHomeApi {
public:
  QList<PendingResult<QList<ScenarioObject>>> loads;
  QList<PendingResult<void>> executions;

  void GetScenarios(QObject* context, ApiResultHandler<QList<ScenarioObject>> handler) override {
    loads.append({context, std::move(handler), {}});
  }
  void ExecuteScenario(const QString& id, QObject* context, ApiResultHandler<void> handler) override {
    executions.append({context, std::move(handler), id});
  }
  void GetUserInfo(QObject*, ApiResultHandler<UserInfo>) override { QFAIL("Unexpected user-info request"); }
  void GetDeviceInfo(const QString&, QObject*, ApiResultHandler<DeviceInfo>) override { QFAIL("Unexpected device request"); }
  void PerformActions(const QList<DeviceActionsObject>&, QObject*, ApiResultHandler<void>) override { QFAIL("Unexpected action request"); }
};

static ScenarioObject Scenario(const QString& id, bool active = true) {
  ScenarioObject scenario;
  scenario.id = id;
  scenario.name = "Scenario " + id;
  scenario.is_active = active;
  return scenario;
}

class ScenariosTests : public QObject {
  Q_OBJECT
private slots:
  void initTestCase() {
#ifdef SCENARIOS_QML_TESTS
    QQuickStyle::setStyle("Basic");
    qRegisterMetaType<QList<ScenarioObject>>();
    qmlRegisterUncreatableType<ScenariosViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "ScenariosViewModel", "Supplied by the application");
    qmlRegisterUncreatableType<ScenariosModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "ScenariosModel", "Supplied by the view model");
#endif
  }

  void LoadingAndRefreshState() {
    FakeHomeApi api;
    ScenarioService service(&api);
    ScenariosViewModel view_model(&service);
    auto* model = view_model.GetScenarios();
    QAbstractItemModelTester model_tester(model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QSignalSpy counts(model, &ScenariosModel::countChanged);

    QCOMPARE(service.GetLoadState(), ScenarioService::LoadState::NotLoaded);
    QVERIFY(view_model.IsLoading());
    QCOMPARE(model->Count(), 0);
    view_model.EnsureLoaded();
    view_model.EnsureLoaded();
    view_model.Refresh();
    QCOMPARE(api.loads.size(), 1);
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Loading);

    api.loads.last().Send(QList<ScenarioObject>{Scenario("a"), Scenario("b", false)});
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Ready);
    QVERIFY(!view_model.IsLoading());
    QCOMPARE(model->Count(), 2);
    QCOMPARE(counts.count(), 1);
    QCOMPARE(model->data(model->index(0), ScenariosModel::IdRole).toString(), QString("a"));
    QCOMPARE(model->data(model->index(1), ScenariosModel::IsActiveRole).toBool(), false);
    QVERIFY(!model->data(QModelIndex(), ScenariosModel::NameRole).isValid());
    QCOMPARE(model->rowCount(model->index(0)), 0);

    view_model.EnsureLoaded();
    QCOMPARE(api.loads.size(), 1);
    view_model.Refresh();
    QCOMPARE(api.loads.size(), 2);
    QVERIFY(view_model.IsLoading());
    api.loads.last().Send(QList<ScenarioObject>{});
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Ready);
    QCOMPARE(model->Count(), 0);
    QCOMPARE(counts.count(), 2);
  }

  void LoadingFailureAndRetry() {
    FakeHomeApi api;
    ScenarioService service(&api);
    ScenariosViewModel view_model(&service);
    view_model.EnsureLoaded();
    const auto failed_request = api.loads.last();
    failed_request.Send(std::unexpected(ApiError{ApiErrorKind::Network, "network failure"}));
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Error);
    QVERIFY(!view_model.IsLoading());
    view_model.Refresh();
    QCOMPARE(api.loads.size(), 2);
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Loading);

    // A duplicated failure from the previous request cannot fail the retry.
    failed_request.Send(std::unexpected(ApiError{ApiErrorKind::Network, "late failure"}));
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Loading);
    api.loads.last().Send(QList<ScenarioObject>{});
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Ready);
  }

  void ExecutionGuardsAndFailure() {
    FakeHomeApi api;
    ScenarioService service(&api);
    ScenariosViewModel view_model(&service);
    QSignalSpy errors(&view_model, &ScenariosViewModel::executionFailed);
    view_model.ExecuteScenario("a");
    QVERIFY(api.executions.isEmpty());
    view_model.EnsureLoaded();
    api.loads.last().Send(QList<ScenarioObject>{Scenario("a"), Scenario("inactive", false)});

    view_model.ExecuteScenario("missing");
    view_model.ExecuteScenario("inactive");
    QVERIFY(api.executions.isEmpty());
    view_model.ExecuteScenario("a");
    view_model.ExecuteScenario("a");
    QCOMPARE(api.executions.size(), 1);
    QCOMPARE(api.executions.last().scenario_id, QString("a"));
    QVERIFY(service.IsExecuting("a"));
    auto* model = view_model.GetScenarios();
    QVERIFY(model->data(model->index(0), ScenariosModel::IsWaitingResponseRole).toBool());

    const auto request = api.executions.last();
    request.Send(std::unexpected(ApiError{ApiErrorKind::Service, "backend details"}));
    QVERIFY(!service.IsExecuting("a"));
    QVERIFY(!model->data(model->index(0), ScenariosModel::IsWaitingResponseRole).toBool());
    QCOMPARE(errors.count(), 1);
    QCOMPARE(errors.at(0).at(0).toString(), QString::fromUtf8("Не удалось выполнить сценарий"));
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Ready);
    request.Send(std::unexpected(ApiError{ApiErrorKind::Service, "duplicate completion"}));
    QCOMPARE(errors.count(), 1);
  }

  void RefreshDuringExecutionUsesStableIds() {
    FakeHomeApi api;
    ScenarioService service(&api);
    ScenariosViewModel view_model(&service);
    auto* model = view_model.GetScenarios();
    QAbstractItemModelTester model_tester(model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    view_model.EnsureLoaded();
    api.loads.last().Send(QList<ScenarioObject>{Scenario("a"), Scenario("b")});
    view_model.ExecuteScenario("a");
    const auto a_request = api.executions.last();

    view_model.Refresh();
    api.loads.last().Send(QList<ScenarioObject>{Scenario("b"), Scenario("a")});
    QVERIFY(model->data(model->index(1), ScenariosModel::IsWaitingResponseRole).toBool());
    QVERIFY(!model->data(model->index(0), ScenariosModel::IsWaitingResponseRole).toBool());
    view_model.ExecuteScenario("b");
    const auto b_request = api.executions.last();
    a_request.Send(ApiResult<void>{});
    QVERIFY(!model->data(model->index(1), ScenariosModel::IsWaitingResponseRole).toBool());
    QVERIFY(model->data(model->index(0), ScenariosModel::IsWaitingResponseRole).toBool());
    b_request.Send(ApiResult<void>{});
    QVERIFY(!model->data(model->index(0), ScenariosModel::IsWaitingResponseRole).toBool());
  }

  void RemovedScenarioStillCompletes() {
    FakeHomeApi api;
    ScenarioService service(&api);
    ScenariosViewModel view_model(&service);
    view_model.EnsureLoaded();
    api.loads.last().Send(QList<ScenarioObject>{Scenario("a")});
    view_model.ExecuteScenario("a");
    const auto request = api.executions.last();
    view_model.Refresh();
    api.loads.last().Send(QList<ScenarioObject>{});
    request.Send(ApiResult<void>{});
    QVERIFY(!service.IsExecuting("a"));
    QCOMPARE(view_model.GetScenarios()->Count(), 0);
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Ready);
  }

  void ResetDiscardsPreviousSessionResponses() {
    FakeHomeApi api;
    ScenarioService service(&api);
    ScenariosViewModel view_model(&service);
    QSignalSpy errors(&view_model, &ScenariosViewModel::executionFailed);
    view_model.EnsureLoaded();
    const auto old_load = api.loads.last();
    old_load.Send(QList<ScenarioObject>{Scenario("a")});
    view_model.ExecuteScenario("a");
    const auto old_execution = api.executions.last();
    view_model.Refresh();
    const auto old_refresh = api.loads.last();
    service.Reset();
    QCOMPARE(view_model.GetScenarios()->Count(), 0);
    QVERIFY(!service.IsExecuting("a"));
    view_model.EnsureLoaded();
    const auto new_load = api.loads.last();

    old_refresh.Send(QList<ScenarioObject>{Scenario("old-account")});
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Loading);
    QCOMPARE(view_model.GetScenarios()->Count(), 0);
    new_load.Send(QList<ScenarioObject>{Scenario("a")});
    view_model.ExecuteScenario("a");
    const auto new_execution = api.executions.last();
    old_execution.Send(ApiResult<void>{});
    old_execution.Send(std::unexpected(ApiError{ApiErrorKind::Service, "old error"}));
    QVERIFY(service.IsExecuting("a"));
    QCOMPARE(errors.count(), 0);
    new_execution.Send(ApiResult<void>{});
    QVERIFY(!service.IsExecuting("a"));
  }

  void NewViewModelReflectsExistingState() {
    FakeHomeApi api;
    ScenarioService service(&api);
    service.EnsureLoaded();
    api.loads.last().Send(QList<ScenarioObject>{Scenario("a")});
    service.ExecuteScenario("a");
    ScenariosViewModel view_model(&service);
    view_model.EnsureLoaded();
    QCOMPARE(api.loads.size(), 1);
    QCOMPARE(view_model.GetState(), ScenariosViewModel::Ready);
    QVERIFY(view_model.GetScenarios()->data(view_model.GetScenarios()->index(0),
      ScenariosModel::IsWaitingResponseRole).toBool());
  }

#ifdef SCENARIOS_QML_TESTS
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
      {"viewModel", QVariant::fromValue(&view_model)}
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
#endif
};

#ifdef SCENARIOS_QML_TESTS
QTEST_MAIN(ScenariosTests)
#else
QTEST_GUILESS_MAIN(ScenariosTests)
#endif
#include "ScenariosTests.moc"
