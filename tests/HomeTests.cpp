#include <QAbstractItemModelTester>
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <memory>
#ifdef HOME_QML_TESTS
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include "QmlTestEnvironment.h"
#endif
#include "models/HomeViewModel.h"
#include "models/DevicesModel/DevicesFilterModel.h"

class HomeTestApi final : public IHomeApi {
public:
  struct Request {
    QPointer<QObject> context;
    ApiResultHandler<UserInfo> handler;
    void Reply(ApiResult<UserInfo> result) const { if (context) { handler(std::move(result)); } }
  };
  QList<Request> requests;
  void GetUserInfo(QObject* context, ApiResultHandler<UserInfo> handler) override {
    requests.append({context, std::move(handler)});
  }
  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>>) override { QFAIL("Unexpected scenario request"); }
  void GetDeviceInfo(const QString&, QObject*, ApiResultHandler<DeviceInfo>) override { QFAIL("Unexpected device request"); }
  void ExecuteScenario(const QString&, QObject*, ApiResultHandler<void>) override { QFAIL("Unexpected execution"); }
  void PerformActions(const QList<DeviceActionsObject>&, QObject*, ApiResultHandler<void>) override { QFAIL("Unexpected action"); }
};

static UserInfo Snapshot() {
  UserInfo info{};
  info.status = Status::Ok;
  for (const auto& id : {QString("one"), QString("two")}) {
    HouseholdObject household{};
    household.id = id; household.name = "House " + id;
    info.households.append(household);
    RoomObject room{};
    room.id = "room-" + id; room.name = "Room " + id; room.household_id = id;
    info.rooms.append(room);
    DeviceObject device{};
    device.id = "device-" + id; device.name = "Lamp " + id; device.type = "devices.types.light";
    device.room = room.id; device.household_id = id;
    info.devices.append(device);
  }
  return info;
}

class HomeTests final : public QObject {
  Q_OBJECT
private slots:
  void LoadsOneSnapshotAndFiltersBySelection() {
    HomeTestApi api;
    HomeService service(&api);
    HomeViewModel view_model(&service);
    QAbstractItemModelTester devices(view_model.GetDevices(), QAbstractItemModelTester::FailureReportingMode::QtTest);
    QAbstractItemModelTester rooms(view_model.GetRooms(), QAbstractItemModelTester::FailureReportingMode::QtTest);
    QAbstractItemModelTester households(view_model.GetHouseholds(), QAbstractItemModelTester::FailureReportingMode::QtTest);
    QSignalSpy counts(view_model.GetHouseholds(), &HouseholdsModel::countChanged);
    QVERIFY(view_model.IsLoading());
    view_model.EnsureLoaded(); view_model.EnsureLoaded(); view_model.Refresh();
    QCOMPARE(api.requests.size(), 1);
    api.requests.last().Reply(Snapshot());
    QCOMPARE(view_model.GetState(), HomeViewModel::Ready);
    QCOMPARE(view_model.GetDevices()->rowCount(), 2);
    QCOMPARE(view_model.GetHouseholds()->GetCount(), 2);
    QCOMPARE(counts.size(), 1);
    QCOMPARE(view_model.GetCurrentHousehold(), QString("one"));
    QCOMPARE(view_model.GetRooms()->rowCount(), 1);
    QCOMPARE(view_model.GetRooms()->data(view_model.GetRooms()->index(0, 0), RoomsModel::IdRole).toString(), QString("room-one"));
    DevicesFilterModel filter;
    filter.setSourceModel(view_model.GetDevices());
    filter.setHouseholdId("one"); filter.setRoomId("room-one");
    QCOMPARE(filter.GetCount(), 1);
    QCOMPARE(filter.data(filter.index(0, 0), DevicesModel::IdRole).toString(), QString("device-one"));
    view_model.SelectHousehold("two");
    QCOMPARE(view_model.GetCurrentHouseholdName(), QString("House two"));
    QCOMPARE(view_model.GetRooms()->data(view_model.GetRooms()->index(0, 0), RoomsModel::IdRole).toString(), QString("room-two"));
    view_model.SelectHousehold("missing");
    QCOMPARE(view_model.GetCurrentHousehold(), QString("two"));
    view_model.EnsureLoaded();
    QCOMPARE(api.requests.size(), 1);
  }

  void RefreshPreservesSelectionAndUpdatesNames() {
    HomeTestApi api; HomeService service(&api); HomeViewModel view_model(&service);
    view_model.EnsureLoaded(); api.requests.last().Reply(Snapshot());
    view_model.SelectHousehold("two");
    QSignalSpy selection(&view_model, &HomeViewModel::currentHouseholdChanged);
    view_model.Refresh();
    auto renamed = Snapshot(); renamed.households[1].name = "Renamed";
    api.requests.last().Reply(renamed);
    QCOMPARE(view_model.GetCurrentHousehold(), QString("two"));
    QCOMPARE(view_model.GetCurrentHouseholdName(), QString("Renamed"));
    QCOMPARE(selection.size(), 1);
    view_model.Refresh(); renamed.households.removeLast(); renamed.rooms.removeLast();
    api.requests.last().Reply(renamed);
    QCOMPARE(view_model.GetCurrentHousehold(), QString("one"));
    QCOMPARE(view_model.GetRooms()->rowCount(), 1);
    view_model.Refresh(); api.requests.last().Reply(UserInfo{});
    QCOMPARE(view_model.GetCurrentHousehold(), QString());
    QCOMPARE(view_model.GetCurrentHouseholdName(), QString());
    QCOMPARE(view_model.GetRooms()->rowCount(), 0);
    QCOMPARE(view_model.GetHouseholds()->GetCount(), 0);
  }

  void FailureAndLateRepliesDoNotCorruptRetry() {
    HomeTestApi api; HomeService service(&api); HomeViewModel view_model(&service);
    view_model.EnsureLoaded(); const auto first = api.requests.last();
    first.Reply(std::unexpected(ApiError{ApiErrorKind::Network, "offline"}));
    QCOMPARE(view_model.GetState(), HomeViewModel::Error);
    QVERIFY(!view_model.IsLoading());
    view_model.Refresh();
    first.Reply(Snapshot());
    QCOMPARE(view_model.GetState(), HomeViewModel::Loading);
    QCOMPARE(view_model.GetDevices()->rowCount(), 0);
    api.requests.last().Reply(Snapshot());
    QCOMPARE(view_model.GetState(), HomeViewModel::Ready);
  }

  void ResetAndNewViewModelUseCurrentSession() {
    HomeTestApi api; HomeService service(&api); HomeViewModel view_model(&service);
    service.EnsureLoaded(); api.requests.last().Reply(Snapshot()); service.SelectHousehold("two");
    HomeViewModel recreated(&service);
    QCOMPARE(recreated.GetCurrentHousehold(), QString("two"));
    QCOMPARE(recreated.GetDevices()->rowCount(), 2);
    recreated.EnsureLoaded(); QCOMPARE(api.requests.size(), 1);
    view_model.Refresh(); const auto old = api.requests.last();
    service.Reset();
    QCOMPARE(view_model.GetDevices()->rowCount(), 0);
    QCOMPARE(view_model.GetHouseholds()->GetCount(), 0);
    QCOMPARE(view_model.GetCurrentHousehold(), QString());
    view_model.EnsureLoaded();
    old.Reply(Snapshot());
    QCOMPARE(view_model.GetState(), HomeViewModel::Loading);
    QCOMPARE(view_model.GetDevices()->rowCount(), 0);
    api.requests.last().Reply(UserInfo{});
    QCOMPARE(view_model.GetState(), HomeViewModel::Ready);
    QCOMPARE(view_model.GetDevices()->rowCount(), 0);
  }

#ifdef HOME_QML_TESTS
  void PageStateAndHouseholdClicksReachViewModel() {
    QQuickStyle::setStyle("Basic");
    HomeTestApi api; HomeService service(&api); HomeViewModel view_model(&service);
    QQmlEngine engine;
    initializeQmlTestEnvironment(&engine);
    engine.addImportPath(HOME_IMPORT_PATH);
    engine.rootContext()->setContextProperty("homeViewModel", &view_model);
    QSignalSpy warnings(&engine, &QQmlEngine::warnings);
    QQmlComponent component(&engine);
    component.loadFromModule("YandexHomeDesktop.Pages", "MainPage");
    QTRY_VERIFY(component.status() != QQmlComponent::Loading);
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    std::unique_ptr<QObject> page(component.create());
    QVERIFY2(page, qPrintable(component.errorString()));
    auto* item = qobject_cast<QQuickItem*>(page.get()); QVERIFY(item);
    QQuickWindow window; window.resize(350, 460);
    item->setParentItem(window.contentItem()); item->setWidth(350); item->setHeight(460); window.show();
    auto* states = page->findChild<QObject*>("homeStates");
    auto* refresh = page->findChild<QQuickItem*>("homeRefreshButton");
    auto* picker = page->findChild<QQuickItem*>("householdPicker");
    QVERIFY(states); QVERIFY(refresh); QVERIFY(picker);
    QCOMPARE(api.requests.size(), 1);
    QCOMPARE(states->property("currentIndex").toInt(), 0);
    QVERIFY(!refresh->isEnabled());
    api.requests.last().Reply(std::unexpected(ApiError{ApiErrorKind::Network, "offline"}));
    QCOMPARE(states->property("currentIndex").toInt(), 1);
    QVERIFY(refresh->isEnabled());
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
      refresh->mapToScene(QPointF(refresh->width() - 20, refresh->height() / 2)).toPoint());
    QCOMPARE(api.requests.size(), 2);
    api.requests.last().Reply(Snapshot());
    QCOMPARE(states->property("currentIndex").toInt(), 2);
    QVERIFY(!picker->property("loading").toBool());
    QVERIFY(QMetaObject::invokeMethod(picker, "open"));
    QQuickItem* row = nullptr;
    QTRY_VERIFY((row = FindItem(picker, "household-two")) != nullptr);
    QTest::qWait(350); // Allow the sheet's existing opening animation to complete.
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
      row->mapToScene(QPointF(row->width() / 2, row->height() / 2)).toPoint());
    QCOMPARE(view_model.GetCurrentHousehold(), QString("two"));
    QVERIFY(!picker->property("opened").toBool());
    QCOMPARE(warnings.size(), 0);
  }
private:
  static QQuickItem* FindItem(QQuickItem* item, const QString& name) {
    if (item->objectName() == name) { return item; }
    for (auto* child : item->childItems()) {
      if (auto* result = FindItem(child, name)) { return result; }
    }
    return nullptr;
  }
#endif
};
#ifdef HOME_QML_TESTS
QTEST_MAIN(HomeTests)
#else
QTEST_GUILESS_MAIN(HomeTests)
#endif
#include "HomeTests.moc"
