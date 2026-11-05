#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <algorithm>
#include <memory>
#include "../support/HomeTestFixtures.h"

using namespace home_test;

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
};

QTEST_GUILESS_MAIN(HomeTests)
#include "HomeTests.moc"
