#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include "models/SettingsTransferModel.h"
#include "models/KeybindSettingsModel.h"
#include "models/HomeViewModel.h"
#include "models/AccountModel.h"
#include "serialization/Serialization.h"

class SettingsTransferTests final : public QObject {
  Q_OBJECT
  static KeybindTarget Target(const QString& device) {
    KeybindTarget target{};
    target.device_id = device;
    target.capability_type = "devices.capabilities.on_off";
    target.instance = "on";
    target.action = "toggle";
    return target;
  }
  static QByteArray Backup() {
    Settings source(nullptr, true);
    source.SetCurrentTheme(1);
    source.SetTrayModeEnabled(true);
    source.SetShowKeybinds(false);
    source.SetFavoriteDevices("account", {"lamp", "socket"});
    source.SetCollapsedRooms("account", {"room"});
    source.SetFavoritesCollapsed("account", true);
    if (source.SaveKeybind("account", Target("lamp"), Qt::Key_F6, "Lamp", "On/off") !=
        Settings::KeybindSaveResult::Saved)
      qFatal("Invalid test binding");
    return *source.ExportSettings("account");
  }
  static bool Write(const QString& path, const QByteArray& data) {
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
  }
private slots:
  void RoundTripReplacesCurrentAccountAndPreservesOtherSettings() {
    QTemporaryDir dir;
    const auto path = dir.filePath("settings.ini");
    Settings settings(path);
    settings.SetFavoriteDevices("other", {"other-device"});
    settings.SetRestEnabled(true);
    settings.SetRestPort(9999);
    QCOMPARE(settings.SaveKeybind("account", Target("old-device"), Qt::Key_F6, "Old", "On/off"),
             Settings::KeybindSaveResult::Saved);
    QSettings raw(path, QSettings::IniFormat);
    raw.setValue("auth/access_token", "synthetic-credential");
    raw.sync();
    KeybindSettingsModel keys(&settings, nullptr);
    keys.SetAccountId("account");
    QSignalSpy preferences(&settings, &Settings::accountPreferencesChanged);
    QSignalSpy theme(&settings, &Settings::currentThemeChanged);
    QCOMPARE(settings.ImportSettings("account", Backup()), Settings::ImportResult::Imported);
    QCOMPARE(theme.count(), 1);
    QCOMPARE(preferences.count(), 1);
    QCOMPARE(keys.GetCombination(Serialization::To(Target("lamp")).toVariantMap()),
             int(Qt::Key_F6));
    Settings restored(path);
    QCOMPARE(restored.GetCurrentTheme(), 1);
    QCOMPARE(restored.GetTrayModeEnabled(), true);
    QCOMPARE(restored.GetShowKeybinds(), false);
    QCOMPARE(restored.GetFavoriteDevices("account"), QStringList({"lamp", "socket"}));
    QCOMPARE(restored.GetCollapsedRooms("account"), QStringList({"room"}));
    QCOMPARE(restored.GetFavoritesCollapsed("account"), true);
    QCOMPARE(restored.GetFavoriteDevices("other"), QStringList({"other-device"}));
    QCOMPARE(restored.GetRestEnabled(), true);
    QCOMPARE(restored.GetRestPort(), quint16(9999));
    QCOMPARE(restored.ExportSettings("account"), std::optional<QByteArray>(Backup()));
    QVERIFY(!restored.ExportSettings({}));
    const auto exported = *restored.ExportSettings("account");
    QVERIFY(!exported.contains("other-device"));
    QVERIFY(!exported.contains("rest"));
    QVERIFY(!exported.contains("token"));
    QVERIFY(!exported.contains("synthetic-credential"));
    QVERIFY(!exported.contains("old-device"));
    QCOMPARE(raw.value("auth/access_token").toString(), QString("synthetic-credential"));
  }

  void InvalidImportsLeavePreferencesUnchanged() {
    Settings settings(nullptr, true);
    const auto before = *settings.ExportSettings("account");
    const auto valid = QJsonDocument::fromJson(Backup()).object();
    QList<QByteArray> invalid{"not JSON", "[]"};
    for (int variant = 0; variant < 9; ++variant) {
      auto object = valid;
      auto app = object["app"].toObject();
      auto account = object["account"].toObject();
      if (variant == 0)
        object["version"] = 2;
      if (variant == 1)
        app.remove("theme");
      if (variant == 2)
        app["tray_mode"] = "true";
      if (variant == 3)
        app["theme"] = 0.5;
      if (variant == 4)
        app["theme"] = 2;
      if (variant == 5)
        account["favorite_devices"] = QJsonArray{"lamp", "lamp"};
      if (variant == 6) {
        auto bindings = account["keybinds"].toArray();
        bindings.append(bindings.first());
        account["keybinds"] = bindings;
      }
      if (variant == 7)
        account["keybinds"] = QJsonArray{QJsonObject{}};
      if (variant == 8)
        account["collapsed_rooms"] = QJsonArray{false};
      object["app"] = app;
      object["account"] = account;
      invalid.append(QJsonDocument(object).toJson());
    }
    QSignalSpy changed(&settings, &Settings::accountPreferencesChanged);
    for (const auto& data : invalid) {
      QCOMPARE(settings.ImportSettings("account", data), Settings::ImportResult::InvalidData);
      QCOMPARE(*settings.ExportSettings("account"), before);
    }
    QCOMPARE(settings.ImportSettings("another", Backup()), Settings::ImportResult::AccountMismatch);
    QCOMPARE(changed.count(), 0);
  }

  void ConfirmationCancellationAndAccountChanges() {
    QTemporaryDir dir;
    const auto path = dir.filePath("backup.json");
    QVERIFY(Write(path, Backup()));
    Settings settings(nullptr, true);
    SettingsTransferService service(&settings);
    SettingsTransferModel model(&service, nullptr);
    QVERIFY(!model.PrepareImport(QUrl::fromLocalFile(path)));
    model.SetAccountId("account");
    QVERIFY(model.PrepareImport(QUrl::fromLocalFile(path)));
    QCOMPARE(settings.GetCurrentTheme(), 0);
    model.CancelImport();
    QVERIFY(!model.ConfirmImport());
    QCOMPARE(settings.GetCurrentTheme(), 0);
    QVERIFY(model.PrepareImport(QUrl::fromLocalFile(path)));
    model.SetAccountId("other");
    QVERIFY(!model.IsImportPending());
    QVERIFY(!model.ConfirmImport());
    model.SetAccountId("account");
    QVERIFY(model.PrepareImport(QUrl::fromLocalFile(path)));
    QVERIFY(Write(path, "invalid after preview"));
    QVERIFY(model.ConfirmImport()); // Apply the already validated snapshot.
    QCOMPARE(settings.GetCurrentTheme(), 1);
    QVERIFY(!model.IsImportPending());
    QVERIFY(!model.HasFailed());
  }

  void FileErrorsAndAtomicExport() {
    QTemporaryDir dir;
    Settings settings(nullptr, true);
    SettingsTransferService service(&settings);
    SettingsTransferModel model(&service, nullptr);
    model.SetAccountId("account");
    const auto file = QUrl::fromLocalFile(dir.filePath("backup.json"));
    QVERIFY(model.Export(file));
    QVERIFY(model.PrepareImport(file));
    model.CancelImport();
    QVERIFY(!model.Export(QUrl::fromLocalFile(dir.path())));
    QVERIFY(model.HasFailed());
    QVERIFY(!model.PrepareImport(QUrl::fromLocalFile(dir.filePath("missing.json"))));
    QVERIFY(!model.Export(QUrl("https://example.invalid/settings.json")));
    QFile output(file.toLocalFile());
    QVERIFY(output.open(QIODevice::ReadOnly));
    QCOMPARE(output.readAll(), *settings.ExportSettings("account"));
  }

  void StorageFailurePreservesValuesAndNotifications() {
    QTemporaryDir dir;
    const auto path = dir.filePath("blocked.ini");
    QVERIFY(QDir().mkdir(path));
    Settings settings(path);
    QSignalSpy changed(&settings, &Settings::accountPreferencesChanged);
    QCOMPARE(settings.ImportSettings("account", Backup()), Settings::ImportResult::StorageError);
    QCOMPARE(settings.GetCurrentTheme(), 0);
    QCOMPARE(settings.GetTrayModeEnabled(), false);
    QVERIFY(settings.GetFavoriteDevices("account").isEmpty());
    QVERIFY(settings.GetKeybinds("account")->isEmpty());
    QCOMPARE(changed.count(), 0);
  }

  void ImportReloadsHomePreferencesImmediately() {
    class Api final : public IHomeApi {
      void GetUserInfo(QObject*, ApiResultHandler<UserInfo> handler) override {
        UserInfo home{};
        home.status = Status::Ok;
        home.devices = {DeviceObject{.id = "lamp", .household_id = "house"}};
        home.households = {HouseholdObject{.id = "house", .name = "House"}};
        handler(home);
      }
      void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>>) override {}
      void GetDeviceInfo(const QString&, QObject*, ApiResultHandler<DeviceInfo>) override {}
      void ExecuteScenario(const QString&, QObject*, ApiResultHandler<void>) override {}
      void PerformActions(const QList<DeviceActionsObject>&, QObject*,
                          ApiResultHandler<void>) override {}
    } api;
    class AccountApi final : public IAccountApi {
      void LoadData(QObject*, ApiResultHandler<AccountInfo> handler) override {
        handler(AccountInfo{{}, {}, {}, "account"});
      }
    } accountApi;
    Settings settings(nullptr, true);
    AccountModel account(&accountApi);
    account.LoadData();
    HomeService service(&api);
    HomeViewModel home(&service, &settings, &account);
    service.EnsureLoaded();
    service.SelectHousehold("house");
    QCOMPARE(home.AreFavoritesCollapsed(), false);
    QCOMPARE(settings.ImportSettings("account", Backup()), Settings::ImportResult::Imported);
    QCOMPARE(home.AreFavoritesCollapsed(), true);
    QCOMPARE(home.GetFavorites()->rowCount(), 1);
  }
};
QTEST_GUILESS_MAIN(SettingsTransferTests)
#include "SettingsTransferTests.moc"
