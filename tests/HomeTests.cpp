#include <QAbstractItemModelTester>
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <algorithm>
#include <memory>
#ifdef HOME_QML_TESTS
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtQuickTest/quicktest.h>
#include "QmlTestEnvironment.h"
#endif
#include "models/HomeViewModel.h"
#include "models/AccountModel.h"
#include "models/DevicesModel/DevicesFilterModel.h"
#include "utils/Settings.h"

class HomeTestAccountApi final : public IAccountApi {
public:
  struct Request {
    QPointer<QObject> context;
    ApiResultHandler<AccountInfo> handler;
    void Reply(ApiResult<AccountInfo> result) const { if (context) { handler(std::move(result)); } }
  };
  QList<Request> requests;
  void LoadData(QObject* context, ApiResultHandler<AccountInfo> handler) override {
    requests.append({context, std::move(handler)});
  }
};

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
  void RoomsHaveStableLexicographicOrder() {
    HomeTestApi api; HomeService service(&api); HomeViewModel view_model(&service);
    QAbstractItemModelTester rooms(view_model.GetRooms(), QAbstractItemModelTester::FailureReportingMode::QtTest);
    auto snapshot = Snapshot();
    snapshot.rooms.clear();
    for (const auto& item : {QPair<QString, QString>{"z", "Спальня"}, {"b", "Кухня"},
                            {"a", "Кухня"}, {"c", "Ванная"}}) {
      RoomObject room{}; room.id = item.first; room.name = item.second; room.household_id = "one";
      snapshot.rooms.append(room);
    }
    RoomObject other{}; other.id = "other"; other.name = "AAA"; other.household_id = "two";
    snapshot.rooms.append(other);
    auto ids = [&view_model] {
      QStringList result;
      auto* rooms = view_model.GetRooms();
      for (int row = 0; row < rooms->rowCount(); ++row) {
        result.append(rooms->data(rooms->index(row, 0), RoomsModel::IdRole).toString());
      }
      return result;
    };
    view_model.EnsureLoaded(); api.requests.last().Reply(snapshot);
    QCOMPARE(ids(), (QStringList{"c", "a", "b", "z"}));
    std::reverse(snapshot.rooms.begin(), snapshot.rooms.end());
    view_model.Refresh(); api.requests.last().Reply(snapshot);
    QCOMPARE(ids(), (QStringList{"c", "a", "b", "z"}));
    for (auto& room : snapshot.rooms) {
      if (room.id == "z") { room.name = "Балкон"; }
    }
    view_model.Refresh(); api.requests.last().Reply(snapshot);
    QCOMPARE(ids(), (QStringList{"z", "c", "a", "b"}));
    view_model.SelectHousehold("two"); QCOMPARE(ids(), QStringList{"other"});
    view_model.SelectHousehold("one"); QCOMPARE(ids(), (QStringList{"z", "c", "a", "b"}));
  }

  void FavoritesKeepAdditionOrderAcrossRefreshAndHouseholds() {
    HomeTestApi api; HomeService service(&api);
    HomeTestAccountApi account_api; AccountModel account(&account_api);
    Settings settings(nullptr, true);
    HomeViewModel view_model(&service, &settings, &account);
    QAbstractItemModelTester favorites(view_model.GetFavorites(), QAbstractItemModelTester::FailureReportingMode::QtTest);
    view_model.EnsureLoaded(); view_model.EnsureLoaded();
    QCOMPARE(account_api.requests.size(), 1);
    account_api.requests.last().Reply(AccountInfo{"User", {}, {}, "account-one"});
    auto snapshot = Snapshot();
    auto extra = snapshot.devices.first(); extra.id = "extra";
    snapshot.devices.append(extra);
    api.requests.last().Reply(snapshot);
    view_model.SetDeviceFavorite("extra", true);
    view_model.SetDeviceFavorite("device-one", true);
    view_model.SetDeviceFavorite("extra", true);
    view_model.SetDeviceFavorite("missing", true);
    view_model.SetRoomCollapsed("missing", true);
    QCOMPARE(settings.GetFavoriteDevices("account-one"), (QStringList{"extra", "device-one"}));
    QVERIFY(settings.GetCollapsedRooms("account-one").isEmpty());
    auto* model = view_model.GetFavorites();
    auto idAt = [model](int row) { return model->data(model->index(row, 0), DevicesModel::IdRole).toString(); };
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(idAt(0), QString("extra"));
    QCOMPARE(idAt(1), QString("device-one"));
    QVERIFY(view_model.GetDevices()->data(view_model.GetDevices()->index(0, 0), DevicesModel::FavoriteRole).toBool());

    view_model.Refresh();
    std::reverse(snapshot.devices.begin(), snapshot.devices.end());
    snapshot.devices[0].name = "Renamed favorite";
    api.requests.last().Reply(snapshot);
    QCOMPARE(idAt(0), QString("extra"));
    QCOMPARE(model->data(model->index(0, 0), DevicesModel::NameRole).toString(), QString("Renamed favorite"));
    view_model.SetDeviceFavorite("extra", false);
    QCOMPARE(model->rowCount(), 1);
    view_model.SetDeviceFavorite("extra", true);
    QCOMPARE(idAt(0), QString("device-one"));
    QCOMPARE(idAt(1), QString("extra"));
    view_model.SetRoomCollapsed("room-one", true);
    view_model.SelectHousehold("two");
    QCOMPARE(model->rowCount(), 0);
    view_model.SetDeviceFavorite("device-two", true);
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(idAt(0), QString("device-two"));
    view_model.SelectHousehold("one");
    QCOMPARE(idAt(0), QString("device-one"));
    QVERIFY(view_model.GetRooms()->data(view_model.GetRooms()->index(0, 0), RoomsModel::CollapsedRole).toBool());

    view_model.Refresh(); snapshot.devices.removeFirst(); api.requests.last().Reply(snapshot);
    QCOMPARE(model->rowCount(), 1);
    QVERIFY(settings.GetFavoriteDevices("account-one").contains("extra"));
    view_model.Refresh(); snapshot.devices.append(extra); api.requests.last().Reply(snapshot);
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(idAt(1), QString("extra"));
  }

  void PreferencesPersistAndStayScopedToStableAccountId() {
    QTemporaryDir dir; QVERIFY(dir.isValid());
    const auto path = dir.filePath("settings.ini");
    {
      Settings settings(path);
      HomeTestApi api; HomeService service(&api);
      HomeTestAccountApi account_api; AccountModel account(&account_api);
      HomeViewModel view_model(&service, &settings, &account);
      view_model.EnsureLoaded(); api.requests.last().Reply(Snapshot());
      QVERIFY(!view_model.ArePreferencesAvailable());
      view_model.SetDeviceFavorite("device-one", true);
      view_model.SetRoomCollapsed("room-one", true);
      QVERIFY(settings.GetFavoriteDevices("").isEmpty());
      account_api.requests.last().Reply(AccountInfo{"Same name", {}, "same@example.com", "account-one"});
      QVERIFY(view_model.ArePreferencesAvailable());
      view_model.SetDeviceFavorite("device-one", true);
      view_model.SetDeviceFavorite("device-two", true);
      view_model.SetRoomCollapsed("room-one", true);
      view_model.SetRoomCollapsed("room-two", true);
      view_model.SetFavoritesCollapsed(true);
      QVERIFY(settings.GetFavoritesCollapsed("account-one"));
    }
    Settings settings(path);
    HomeTestApi api; HomeService service(&api);
    HomeTestAccountApi account_api; AccountModel account(&account_api);
    HomeViewModel view_model(&service, &settings, &account);
    view_model.EnsureLoaded();
    account_api.requests.last().Reply(AccountInfo{"Renamed user", {}, "changed@example.com", "account-one"});
    api.requests.last().Reply(Snapshot());
    QCOMPARE(view_model.GetFavorites()->rowCount(), 1);
    QVERIFY(view_model.GetRooms()->data(view_model.GetRooms()->index(0, 0), RoomsModel::CollapsedRole).toBool());
    QCOMPARE(settings.GetFavoriteDevices("account-one"), (QStringList{"device-one", "device-two"}));
    QVERIFY(view_model.AreFavoritesCollapsed());

    account.LoadData(); const auto stale = account_api.requests.last();
    service.Reset(); account.Reset();
    QVERIFY(!view_model.ArePreferencesAvailable());
    QVERIFY(!view_model.AreFavoritesCollapsed());
    QCOMPARE(view_model.GetFavorites()->rowCount(), 0);
    view_model.EnsureLoaded(); api.requests.last().Reply(Snapshot());
    stale.Reply(AccountInfo{"Old session", {}, {}, "account-one"});
    QVERIFY(!view_model.ArePreferencesAvailable());
    account_api.requests.last().Reply(AccountInfo{"Same name", {}, "same@example.com", "account-two"});
    QCOMPARE(view_model.GetFavorites()->rowCount(), 0);
    QVERIFY(!view_model.GetDevices()->data(view_model.GetDevices()->index(0, 0), DevicesModel::FavoriteRole).toBool());
    QVERIFY(!view_model.AreFavoritesCollapsed());
    QVERIFY(!view_model.GetRooms()->data(view_model.GetRooms()->index(0, 0), RoomsModel::CollapsedRole).toBool());
    view_model.SetDeviceFavorite("device-one", true);
    view_model.SetRoomCollapsed("room-one", true);
    view_model.SetRoomCollapsed("room-one", false);
    QCOMPARE(settings.GetFavoriteDevices("account-two"), QStringList{"device-one"});
    QVERIFY(settings.GetCollapsedRooms("account-two").isEmpty());

    service.Reset(); account.Reset(); view_model.EnsureLoaded();
    account_api.requests.last().Reply(AccountInfo{"User", {}, {}, "account-one"});
    api.requests.last().Reply(Snapshot());
    view_model.SelectHousehold("two");
    QCOMPARE(view_model.GetFavorites()->rowCount(), 1);
    QVERIFY(view_model.GetRooms()->data(view_model.GetRooms()->index(0, 0), RoomsModel::CollapsedRole).toBool());
    QVERIFY(view_model.AreFavoritesCollapsed());
    view_model.SetFavoritesCollapsed(false);
    QVERIFY(!settings.GetFavoritesCollapsed("account-one"));
    QVERIFY(!settings.GetFavoritesCollapsed("account-two"));
  }

  void AccountFailureCanBeRetriedWithoutSharingPreferences() {
    HomeTestApi api; HomeService service(&api);
    HomeTestAccountApi account_api; AccountModel account(&account_api);
    Settings settings(nullptr, true);
    settings.SetFavoriteDevices("account-one", {"device-one"});
    settings.SetCollapsedRooms("account-one", {"room-one"});
    HomeViewModel view_model(&service, &settings, &account);
    view_model.EnsureLoaded(); api.requests.last().Reply(Snapshot());
    account_api.requests.last().Reply(std::unexpected(ApiError{ApiErrorKind::Network, "offline"}));
    QCOMPARE(view_model.GetFavorites()->rowCount(), 0);
    QVERIFY(!view_model.ArePreferencesAvailable());
    view_model.Refresh();
    QCOMPARE(account_api.requests.size(), 2);
    account_api.requests.last().Reply(AccountInfo{"User", {}, {}, "account-one"});
    api.requests.last().Reply(Snapshot());
    QCOMPARE(view_model.GetFavorites()->rowCount(), 1);
    QVERIFY(view_model.GetRooms()->data(view_model.GetRooms()->index(0, 0), RoomsModel::CollapsedRole).toBool());
    AccountModel other_account(&account_api);
    HomeViewModel other_view_model(&service, &settings, &other_account);
    other_view_model.EnsureLoaded();
    account_api.requests.last().Reply(AccountInfo{"No ID", {}, {}, {}});
    QVERIFY(!other_view_model.ArePreferencesAvailable());
    QCOMPARE(other_view_model.GetFavorites()->rowCount(), 0);
    QVERIFY(!other_view_model.GetRooms()->data(other_view_model.GetRooms()->index(0, 0), RoomsModel::CollapsedRole).toBool());
  }

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
  void initTestCase() { QQuickStyle::setStyle("Basic"); }

  void ExpandingLargeSectionKeepsItsHeadingInPlace_data() {
    QTest::addColumn<int>("leading_room_count");
    QTest::addColumn<bool>("favorites");
    QTest::addColumn<int>("viewport_height");
    QTest::newRow("first-room") << 0 << false << 230;
    QTest::newRow("room-after-scrolling") << 12 << false << 230;
    QTest::newRow("room-in-taller-window") << 12 << false << 460;
    QTest::newRow("favorites") << 0 << true << 230;
  }

  void ExpandingLargeSectionKeepsItsHeadingInPlace() {
    QFETCH(int, leading_room_count);
    QFETCH(bool, favorites);
    QFETCH(int, viewport_height);
    HomeTestApi api; HomeService service(&api);
    HomeTestAccountApi account_api; AccountModel account(&account_api);
    Settings settings(nullptr, true);
    QStringList collapsed_rooms{"room-one", "room-tail"};
    for (int i = 0; i < leading_room_count; ++i) { collapsed_rooms.append("leading-" + QString::number(i)); }
    settings.SetCollapsedRooms("account-one", collapsed_rooms);
    settings.SetFavoritesCollapsed("account-one", true);
    HomeViewModel view_model(&service, &settings, &account);
    QQmlEngine engine;
    initializeQmlTestEnvironment(&engine);
    engine.addImportPath(HOME_IMPORT_PATH);
    QSignalSpy warnings(&engine, &QQmlEngine::warnings);
    QQmlComponent component(&engine);
    component.loadFromModule("YandexHomeDesktop.Pages", "DevicesPage");
    QTRY_VERIFY(component.status() != QQmlComponent::Loading);
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    std::unique_ptr<QObject> page(component.createWithInitialProperties({{"viewModel", QVariant::fromValue(&view_model)}}));
    QVERIFY2(page, qPrintable(component.errorString()));
    auto* item = qobject_cast<QQuickItem*>(page.get()); QVERIFY(item);
    QQuickWindow window; window.resize(350, viewport_height);
    item->setParentItem(window.contentItem()); item->setWidth(350); item->setHeight(viewport_height); window.show();
    auto snapshot = Snapshot();
    for (int i = 0; i < leading_room_count; ++i) {
      RoomObject room{}; room.id = "leading-" + QString::number(i);
      room.name = "A room " + QString::number(i); room.household_id = "one";
      snapshot.rooms.append(room);
    }
    for (int i = 0; i < 20; ++i) {
      auto device = snapshot.devices.first(); device.id = "lamp-" + QString::number(i);
      snapshot.devices.append(device);
    }
    RoomObject tail{}; tail.id = "room-tail"; tail.name = "Z other room"; tail.household_id = "one";
    snapshot.rooms.append(tail);
    if (favorites) {
      QStringList favorite_ids;
      for (const auto& device : snapshot.devices) {
        if (device.household_id == "one") { favorite_ids.append(device.id); }
      }
      settings.SetFavoriteDevices("account-one", favorite_ids);
    }
    account_api.requests.last().Reply(AccountInfo{"User", {}, {}, "account-one"});
    api.requests.last().Reply(snapshot);
    QQuickItem* list = nullptr;
    QTRY_VERIFY((list = FindItem(item, "roomsList")) != nullptr);
    auto* favorites_body = FindItem(item, "sectionBody-favorites"); QVERIFY(favorites_body);
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    QTRY_COMPARE(favorites_body->height(), 0.0);
    QVERIFY(list->setProperty("contentY", std::max(0.0, list->property("contentHeight").toDouble() - list->height())));
    const QString section_id = favorites ? "favorites" : "room-one";
    QQuickItem* heading = nullptr;
    QTRY_VERIFY((heading = FindItem(item, "collapse-" + section_id)) != nullptr);
    auto* body = FindItem(item, "sectionBody-" + section_id); QVERIFY(body);
    QTRY_COMPARE(body->height(), 0.0);
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    const auto initial_y = heading->mapToItem(list, QPointF()).y();
    const auto initial_scroll = list->property("contentY").toDouble();
    QVERIFY(initial_y >= 0 && initial_y + heading->height() <= list->height());
    qreal maximum_shift = 0;
    const auto track_heading = [&] {
      maximum_shift = std::max(maximum_shift, qAbs(heading->mapToItem(list, QPointF()).y() - initial_y));
    };
    const auto tracking = connect(&window, &QQuickWindow::afterAnimating, &window, track_heading);
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
      heading->mapToScene(QPointF(heading->width() / 2, heading->height() / 2)).toPoint());
    QTRY_COMPARE(body->height(), body->property("expandedHeight").toDouble());
    QVERIFY(body->height() > list->height());
    QVERIFY2(maximum_shift < 1, qPrintable(QString("Heading moved by %1 pixels during expansion").arg(maximum_shift)));
    QVERIFY(qAbs(heading->mapToItem(list, QPointF()).y() - initial_y) < 1);
    QCOMPARE(list->property("contentY").toDouble(), initial_scroll);
    disconnect(tracking);

    // All rows remain reachable in the same viewport after the animation finishes.
    QVERIFY(list->setProperty("contentY", list->property("contentHeight").toDouble() - list->height()));
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    auto* last_device = FindItem(body, "device-lamp-19"); QVERIFY(last_device);
    const auto last_y = last_device->mapToItem(list, QPointF()).y();
    QVERIFY(last_y >= 0 && last_y + last_device->height() <= list->height());
    QVERIFY(list->setProperty("contentY", initial_scroll));
    maximum_shift = 0;
    const auto collapse_tracking = connect(&window, &QQuickWindow::afterAnimating, &window, track_heading);
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
      heading->mapToScene(QPointF(heading->width() / 2, heading->height() / 2)).toPoint());
    QTRY_COMPARE(body->height(), 0.0);
    QVERIFY2(maximum_shift < 1, qPrintable(QString("Heading moved by %1 pixels during collapse").arg(maximum_shift)));
    QVERIFY(qAbs(heading->mapToItem(list, QPointF()).y() - initial_y) < 1);
    disconnect(collapse_tracking);
    QCOMPARE(warnings.size(), 0);
  }

  void RoomCollapseAndFavoriteClicksReachViewModel() {
    HomeTestApi api; HomeService service(&api);
    HomeTestAccountApi account_api; AccountModel account(&account_api);
    Settings settings(nullptr, true);
    HomeViewModel view_model(&service, &settings, &account);
    QQmlEngine engine;
    initializeQmlTestEnvironment(&engine);
    engine.addImportPath(HOME_IMPORT_PATH);
    QSignalSpy warnings(&engine, &QQmlEngine::warnings);
    QQmlComponent component(&engine);
    component.loadFromModule("YandexHomeDesktop.Pages", "DevicesPage");
    QTRY_VERIFY(component.status() != QQmlComponent::Loading);
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    std::unique_ptr<QObject> page(component.createWithInitialProperties({{"viewModel", QVariant::fromValue(&view_model)}}));
    QVERIFY2(page, qPrintable(component.errorString()));
    auto* item = qobject_cast<QQuickItem*>(page.get()); QVERIFY(item);
    QQuickWindow window; window.resize(350, 460);
    item->setParentItem(window.contentItem()); item->setWidth(350); item->setHeight(460); window.show();
    auto snapshot = Snapshot();
    auto extra = snapshot.devices.first(); extra.id = "extra"; extra.name = QString(200, 'X');
    snapshot.devices.append(extra);
    api.requests.last().Reply(snapshot);
    account_api.requests.last().Reply(AccountInfo{"User", {}, {}, "account-one"});
    QQuickItem* collapse = nullptr;
    QTRY_VERIFY((collapse = FindItem(item, "collapse-room-one")) != nullptr);
    auto* room_devices = FindItem(item, "roomDevices-room-one"); QVERIFY(room_devices);
    auto* room_body = FindItem(item, "sectionBody-room-one"); QVERIFY(room_body);
    auto* arrow = FindItem(collapse, "sectionArrow"); QVERIFY(arrow);
    auto* title = FindItem(collapse, "sectionTitle"); QVERIFY(title);
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    QTRY_VERIFY(room_devices->height() > 0);
    QTRY_COMPARE(room_body->height(), room_body->property("expandedHeight").toDouble());
    QCOMPARE(arrow->x() - title->x() - title->width(), 8.0);
    QVERIFY(arrow->property("background").isNull());
    auto click = [&window](QQuickItem* target) {
      QVERIFY(QQuickTest::qWaitForPolish(&window));
      QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
        target->mapToScene(QPointF(target->width() / 2, target->height() / 2)).toPoint());
    };
    click(collapse);
    QTRY_VERIFY(room_body->height() > 0 && room_body->height() < room_body->property("expandedHeight").toDouble());
    QTRY_VERIFY(arrow->rotation() > 90 && arrow->rotation() < 180);
    QTRY_VERIFY(!room_devices->isVisible());
    QCOMPARE(room_body->height(), 0.0);
    QTRY_COMPARE(arrow->rotation(), 90.0);
    QCOMPARE(settings.GetCollapsedRooms("account-one"), QStringList{"room-one"});
    click(collapse);
    QTRY_VERIFY(room_body->height() > 0 && room_body->height() < room_body->property("expandedHeight").toDouble());
    QTRY_COMPARE(room_body->height(), room_body->property("expandedHeight").toDouble());
    QTRY_COMPARE(arrow->rotation(), 180.0);
    QTRY_VERIFY(room_devices->isVisible());
    QVERIFY(settings.GetCollapsedRooms("account-one").isEmpty());
    auto* first_star = FindItem(room_devices, "favorite-extra"); QVERIFY(first_star);
    click(first_star);
    QCOMPARE(view_model.GetFavorites()->rowCount(), 1);
    QQuickItem* favorites_section = nullptr;
    QTRY_VERIFY((favorites_section = FindItem(item, "favoritesSection")) && favorites_section->isVisible());
    QQuickItem* favorite_star = nullptr;
    QTRY_VERIFY((favorite_star = FindItem(favorites_section, "favorite-extra")) != nullptr);
    auto* rooms_list = FindItem(item, "roomsList"); QVERIFY(rooms_list);
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    QVERIFY(rooms_list->setProperty("contentY", 0.0));
    QTRY_VERIFY(favorite_star->mapToItem(rooms_list, QPointF()).y() >= 0);
    auto* favorites_collapse = FindItem(favorites_section, "collapse-favorites"); QVERIFY(favorites_collapse);
    auto* favorites_body = FindItem(favorites_section, "sectionBody-favorites"); QVERIFY(favorites_body);
    QTRY_COMPARE(favorites_body->height(), favorites_body->property("expandedHeight").toDouble());
    click(favorites_collapse);
    QVERIFY(view_model.AreFavoritesCollapsed());
    QVERIFY(settings.GetFavoritesCollapsed("account-one"));
    QTRY_VERIFY(favorites_body->height() > 0 && favorites_body->height() < favorites_body->property("expandedHeight").toDouble());
    QTRY_COMPARE(favorites_body->height(), 0.0);
    click(favorites_collapse);
    QVERIFY(!view_model.AreFavoritesCollapsed());
    QTRY_COMPARE(favorites_body->height(), favorites_body->property("expandedHeight").toDouble());
    auto* router = qobject_cast<QmlTestModel*>(engine.rootContext()->contextProperty("router").value<QObject*>());
    auto* device_api = qobject_cast<QmlTestDeviceApi*>(engine.rootContext()->contextProperty("deviceTestApi").value<QObject*>());
    QVERIFY(router); QVERIFY(device_api);
    QCOMPARE(router->callCount, 0); // The star does not navigate to device controls.
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    auto* favorite_row = FindItem(favorites_section, "device-extra"); QVERIFY(favorite_row);
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
      favorite_row->mapToScene(QPointF(100, favorite_row->height() / 2)).toPoint());
    QCOMPARE(router->lastRoute, QString("device"));
    QCOMPARE(device_api->property("deviceId").toString(), QString("extra"));
    click(favorite_star);
    QCOMPARE(view_model.GetFavorites()->rowCount(), 0);
    QVERIFY(favorites_section->isVisible());
    auto* empty_message = FindItem(favorites_section, "sectionEmpty-favorites"); QVERIFY(empty_message);
    QTRY_VERIFY(empty_message->isVisible());
    QVERIFY(settings.GetFavoriteDevices("account-one").isEmpty());
    QCOMPARE(warnings.size(), 0);
  }

  void PageStateAndHouseholdClicksReachViewModel() {
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
