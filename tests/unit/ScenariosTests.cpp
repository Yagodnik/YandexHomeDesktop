#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>
#include <memory>
#include "../support/ScenariosTestFixtures.h"

using namespace scenarios_test;

class ScenariosTests : public QObject {
  Q_OBJECT
private slots:
  void OneShotRunsValidateFreshDataWithoutLoadingGuiState() {
    FakeHomeApi api;
    ScenarioService service(&api);
    QObject caller;
    std::optional<CommandResult> result;
    const auto complete = [&result](CommandResult value) { result = std::move(value); };

    service.RunActiveScenario("evening", &caller, complete);
    api.loads.last().Send(QList<ScenarioObject>{Scenario("other")});
    QVERIFY(result && !*result);
    QCOMPARE(std::get<CommandRejection>(result->error()).reason, CommandRejectionReason::ScenarioNotFound);
    QVERIFY(api.executions.isEmpty());

    service.RunActiveScenario("evening", &caller, complete);
    api.loads.last().Send(QList<ScenarioObject>{Scenario("evening", false)});
    QVERIFY(result && !*result);
    QCOMPARE(std::get<CommandRejection>(result->error()).reason, CommandRejectionReason::InactiveScenario);
    QVERIFY(api.executions.isEmpty());

    result.reset();
    service.RunActiveScenario("evening", &caller, complete);
    api.loads.last().Send(QList<ScenarioObject>{Scenario("evening")});
    QVERIFY(!result);
    QCOMPARE(api.executions.last().scenario_id, QString("evening"));
    api.executions.last().Send(std::unexpected(ApiError{ApiErrorKind::Http, "expired", 401}));
    QVERIFY(result && !*result);
    QCOMPARE(std::get<ApiError>(result->error()).http_status, 401);
    QCOMPARE(service.GetLoadState(), ScenarioService::LoadState::NotLoaded);
    QVERIFY(service.GetScenarios().isEmpty());
    QVERIFY(!service.IsExecuting("evening"));
  }

  void OneShotRunsCancelWithTheirCallerOrService() {
    FakeHomeApi api;
    ScenarioService service(&api);
    int deliveries = 0;
    const auto complete = [&deliveries](CommandResult) { ++deliveries; };
    auto caller = std::make_unique<QObject>();
    service.RunActiveScenario("evening", caller.get(), complete);
    caller.reset();
    api.loads.last().Send(QList<ScenarioObject>{Scenario("evening")});
    QVERIFY(api.executions.isEmpty());

    caller = std::make_unique<QObject>();
    service.RunActiveScenario("evening", caller.get(), complete);
    api.loads.last().Send(QList<ScenarioObject>{Scenario("evening")});
    caller.reset();
    api.executions.last().Send(ApiResult<void>{});
    QCOMPARE(deliveries, 0);

    caller = std::make_unique<QObject>();
    auto transient = std::make_unique<ScenarioService>(&api);
    transient->RunActiveScenario("evening", caller.get(), complete);
    transient.reset();
    api.loads.last().Send(QList<ScenarioObject>{Scenario("evening")});
    QCOMPARE(api.executions.size(), 1);
    QCOMPARE(deliveries, 0);
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
};

QTEST_GUILESS_MAIN(ScenariosTests)
#include "ScenariosTests.moc"
