#include <QTest>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include "models/KeybindSettingsModel.h"
#include "models/AccountModel.h"
#include "serialization/Serialization.h"

class KeybindSettingsTests final : public QObject {
  Q_OBJECT
private:
  static KeybindTarget Range(const QString& device, const QString& action = "increase") {
    KeybindTarget target{};
    target.device_id = device;
    target.capability_type = "devices.capabilities.range";
    target.instance = "brightness";
    target.action = action;
    return target;
  }
  static QVariantMap Map(const KeybindTarget& target) {
    return Serialization::To(target).toVariantMap();
  }
  static QString StorageKey(const QString& account) {
    return "accounts/" +
           QCryptographicHash::hash(account.toUtf8(), QCryptographicHash::Sha256).toHex() +
           "/keybinds";
  }
  using Result = Settings::KeybindSaveResult;

private slots:
  void ScenarioIdentityPersistenceAndSharedDeviceConflicts() {
    QTemporaryDir directory;
    const auto path = directory.filePath("settings.ini");
    const KeybindTarget scenario{.action = "run", .scenario_id = "lamp"};
    const KeybindTarget other{.action = "run", .scenario_id = "other"};
    {
      Settings settings(path);
      KeybindSettingsModel model(&settings, nullptr);
      model.SetAccountId("account");
      QVERIFY(model.Save(Map(scenario), Qt::Key_F6, "Evening", "Run scenario"));
      QVERIFY(!model.Save(Map(Range("lamp")), Qt::Key_F6, "Lamp", "Increase"));
      QVERIFY(model.GetLastError().contains("Evening"));
      QVERIFY(!model.Save(Map(other), Qt::Key_F6, "Evening", "Run scenario"));
      QVERIFY(model.Save(Map(Range("lamp")), Qt::Key_F7, "Lamp", "Increase"));
      QVERIFY(model.Save(Map(scenario), Qt::Key_F8, "Renamed", "Run scenario"));
      QVERIFY(model.Save(Map(other), Qt::Key_F6, "Evening", "Run scenario"));
      QCOMPARE(settings.GetKeybinds("account")->size(), 3);
      QCOMPARE(settings.SaveKeybind("other-account", scenario, Qt::Key_F6, "Other", {}),
               Result::Saved);
    }
    Settings restored(path);
    KeybindSettingsModel model(&restored, nullptr);
    model.SetAccountId("account");
    QCOMPARE(model.GetCombination(Map(scenario)), int(Qt::Key_F8));
    QCOMPARE(model.GetCombination(Map(Range("lamp"))), int(Qt::Key_F7));
    Settings imported(nullptr, true);
    QCOMPARE(imported.ImportSettings("account", *restored.ExportSettings("account")),
             Settings::ImportResult::Imported);
    QCOMPARE(imported.GetKeybinds("account")->size(), 3);
    QVERIFY(model.Save(Map(scenario), 0, "Renamed", "Run scenario"));
    QCOMPARE(model.GetCombination(Map(scenario)), 0);
    model.SetAccountId("other-account");
    QCOMPARE(model.GetCombination(Map(scenario)), int(Qt::Key_F6));
  }

  void LegacyDeviceStorageAndBackupsUpgradeWithoutLosingAssignments() {
    QTemporaryDir directory;
    const auto path = directory.filePath("settings.ini");
    Settings settings(path);
    QCOMPARE(settings.SaveKeybind("account", Range("lamp"), Qt::Key_F6, "Lamp", "Increase"),
             Result::Saved);
    QSettings raw(path, QSettings::IniFormat);
    auto document =
        QJsonDocument::fromJson(raw.value(StorageKey("account")).toString().toUtf8()).object();
    auto binding = document["bindings"].toArray()[0].toObject();
    auto target = binding["target"].toObject();
    target.remove("scenario_id");
    binding["target"] = target;
    document["version"] = 1;
    document["bindings"] = QJsonArray{binding};
    raw.setValue(StorageKey("account"), QString::fromUtf8(QJsonDocument(document).toJson()));
    raw.sync();
    QCOMPARE(settings.GetKeybinds("account")->size(), 1);
    QCOMPARE(settings.GetKeybinds("account")->first().combination, int(Qt::Key_F6));
    auto backup = QJsonDocument::fromJson(*settings.ExportSettings("account")).object();
    QCOMPARE(backup["version"].toInt(), 2);
    backup["version"] = 1;
    auto account = backup["account"].toObject();
    account["keybinds"] = QJsonArray{binding};
    backup["account"] = account;
    Settings imported(nullptr, true);
    QCOMPARE(imported.ImportSettings("account", QJsonDocument(backup).toJson()),
             Settings::ImportResult::Imported);
    QCOMPARE(imported.GetKeybinds("account")->first().target.scenario_id, QString{});
    QCOMPARE(imported.GetKeybinds("account")->first().combination, int(Qt::Key_F6));
    const KeybindTarget scenario{.action = "run", .scenario_id = "evening"};
    QCOMPARE(settings.SaveKeybind("account", scenario, Qt::Key_F7, "Evening", "Run"),
             Result::Saved);
    raw.sync();
    QCOMPARE(QJsonDocument::fromJson(raw.value(StorageKey("account")).toString().toUtf8())
                 .object()["version"]
                 .toInt(),
             2);
    QCOMPARE(settings.GetKeybinds("account")->size(), 2);
  }

  void RejectsAmbiguousScenarioTargetsAndMalformedVersionTwo() {
    Settings settings(nullptr, true);
    const KeybindTarget valid{.action = "run", .scenario_id = "evening"};
    QCOMPARE(settings.SaveKeybind("account", valid, Qt::Key_F6, "Evening", "Run"), Result::Saved);
    const auto before = *settings.ExportSettings("account");
    for (int variant = 0; variant < 7; ++variant) {
      auto backup = QJsonDocument::fromJson(before).object();
      auto account = backup["account"].toObject();
      auto binding = account["keybinds"].toArray()[0].toObject();
      auto target = binding["target"].toObject();
      if (variant == 0)
        target["device_id"] = "lamp";
      if (variant == 1)
        target["instance"] = "on";
      if (variant == 2)
        target["action"] = "toggle";
      if (variant == 3)
        target["value"] = "true";
      if (variant == 4)
        target.remove("scenario_id");
      if (variant == 5)
        target["scenario_id"] = 123;
      if (variant == 6)
        backup["version"] = 1;
      binding["target"] = target;
      account["keybinds"] = QJsonArray{binding};
      backup["account"] = account;
      QCOMPARE(settings.ImportSettings("account", QJsonDocument(backup).toJson()),
               Settings::ImportResult::InvalidData);
      QCOMPARE(*settings.ExportSettings("account"), before);
    }
  }

  void PersistsAllActionIdentitiesAcrossRecreation() {
    QTemporaryDir directory;
    const auto path = directory.filePath("settings.ini");
    QList<KeybindTarget> targets{Range("lamp"), Range("lamp", "decrease")};
    for (const auto& type : {"devices.capabilities.on_off", "devices.capabilities.toggle"}) {
      auto target = Range("lamp", "toggle");
      target.capability_type = type;
      target.instance = QString(type).endsWith("on_off") ? "on" : "backlight";
      targets.append(target);
    }
    auto mode = Range("lamp", "set_mode");
    mode.capability_type = "devices.capabilities.mode";
    mode.instance = "fan_speed";
    mode.value = "auto";
    targets.append(mode);
    for (const auto& entry :
         {std::pair{"set_color", "#f2595b"}, {"set_temperature", "2700"}, {"set_scene", "party"}}) {
      auto target = Range("lamp", entry.first);
      target.capability_type = "devices.capabilities.color_setting";
      target.instance.clear();
      target.value = entry.second;
      targets.append(target);
    }
    {
      Settings settings(path);
      for (qsizetype i = 0; i < targets.size(); ++i) {
        QCOMPARE(
            settings.SaveKeybind("account", targets[i], Qt::Key_F1 + i, "Original name", "Action"),
            Result::Saved);
      }
    }
    Settings restored(path);
    KeybindSettingsModel model(&restored, nullptr);
    model.SetAccountId("account");
    QVERIFY(model.IsAvailable());
    for (qsizetype i = 0; i < targets.size(); ++i)
      QCOMPARE(model.GetCombination(Map(targets[i])), Qt::Key_F1 + i);
    QVERIFY(model.Save(Map(targets[0]), Qt::Key_F1, "Renamed device", "Renamed label"));
    QCOMPARE(restored.GetKeybinds("account")->size(), targets.size());
    Settings reopened(path);
    QCOMPARE(reopened.GetKeybinds("account")->last().device_name, QString("Renamed device"));
    QCOMPARE(model.GetCombination(Map(targets[0])), int(Qt::Key_F1));
  }

  void RejectsDuplicatesAndReleasesReplacedOrClearedCombinations() {
    Settings settings(nullptr, true);
    const auto first = Range("lamp");
    const auto second = Range("socket");
    QCOMPARE(settings.SaveKeybind("account", first, Qt::Key_F6, "Desk lamp", "Increase"),
             Result::Saved);
    QCOMPARE(settings.SaveKeybind("account", first, Qt::Key_F6, "Desk lamp", "Increase"),
             Result::Saved);
    QCOMPARE(settings.SaveKeybind("account", second, Qt::Key_F6, "Socket", "Increase"),
             Result::Conflict);
    QCOMPARE(
        settings.SaveKeybind("account", Range("lamp", "decrease"), Qt::Key_F6, "Lamp", "Decrease"),
        Result::Conflict);
    QCOMPARE(settings.GetKeybinds("account")->size(), 1);
    QCOMPARE(settings.SaveKeybind("account", first, Qt::Key_F7, "Lamp", "Increase"), Result::Saved);
    QCOMPARE(settings.SaveKeybind("account", second, Qt::Key_F6, "Socket", "Increase"),
             Result::Saved);
    QCOMPARE(settings.SaveKeybind("account", first, 0, "Lamp", "Increase"), Result::Saved);
    QCOMPARE(
        settings.SaveKeybind("account", Range("lamp", "decrease"), Qt::Key_F7, "Lamp", "Decrease"),
        Result::Saved);
    QCOMPARE(settings.SaveKeybind("other-account", first, Qt::Key_F6, "Lamp", "Increase"),
             Result::Saved);
  }

  void NormalizesKeysAndRejectsInvalidAssignments() {
    QCOMPARE(Settings::NormalizeKeyCombination('k', Qt::ControlModifier),
             Settings::NormalizeKeyCombination(Qt::Key_K, Qt::ControlModifier));
    QCOMPARE(Settings::NormalizeKeyCombination(Qt::Key_Enter, Qt::KeypadModifier),
             int(Qt::Key_Return));
    QCOMPARE(
        Settings::NormalizeKeyCombination(Qt::Key_Tab, Qt::ControlModifier | Qt::KeypadModifier),
        int(Qt::Key_Tab) | int(Qt::ControlModifier));
    for (const auto key :
         {Qt::Key_Control, Qt::Key_Shift, Qt::Key_Alt, Qt::Key_Meta, Qt::Key_AltGr, Qt::Key_Escape,
          Qt::Key_Delete, Qt::Key_Backspace, Qt::Key_Tab, Qt::Key_unknown}) {
      QCOMPARE(Settings::NormalizeKeyCombination(key, 0), 0);
    }
    Settings settings(nullptr, true);
    QCOMPARE(settings.SaveKeybind({}, Range("lamp"), Qt::Key_F6, "Lamp", {}), Result::Invalid);
    QCOMPARE(settings.SaveKeybind("account", Range({}), Qt::Key_F6, "Lamp", {}), Result::Invalid);
    QCOMPARE(settings.SaveKeybind("account", Range("lamp"), Qt::Key_Control, "Lamp", {}),
             Result::Invalid);
    auto target = Range("lamp");
    target.capability_type = "devices.capabilities.video_stream";
    QCOMPARE(settings.SaveKeybind("account", target, Qt::Key_F6, "Lamp", {}), Result::Invalid);
    KeybindSettingsModel model(&settings, nullptr);
    QVERIFY(
        !model.DisplayCombination(model.MakeCombination(Qt::Key_K, Qt::ControlModifier)).isEmpty());
  }

  void AccountLifecycleAndDemoIsolation() {
    class AccountApi final : public IAccountApi {
    public:
      QString id = "one";
      void LoadData(QObject*, ApiResultHandler<AccountInfo> handler) override {
        handler(AccountInfo{"Name", {}, {}, id});
      }
    } api;
    AccountModel account(&api);
    Settings settings(nullptr, true);
    KeybindSettingsModel model(&settings, &account);
    const auto target = Map(Range("lamp"));
    QVERIFY(!model.IsAvailable());
    QVERIFY(!model.Save(target, Qt::Key_F6, "Lamp", "Increase"));
    account.LoadData();
    QVERIFY(model.IsAvailable());
    QVERIFY(model.Save(target, Qt::Key_F6, "Lamp", "Increase"));
    account.Reset();
    QVERIFY(!model.IsAvailable());
    QCOMPARE(model.GetCombination(target), 0);
    QVERIFY(!model.Save(target, Qt::Key_F7, "Lamp", "Increase"));
    api.id = "two";
    account.LoadData();
    QCOMPARE(model.GetCombination(target), 0);
    QVERIFY(model.Save(Map(Range("socket")), Qt::Key_F6, "Socket", "Increase"));
    api.id = "one";
    account.LoadData();
    QCOMPARE(model.GetCombination(target), int(Qt::Key_F6));
    Settings isolated(nullptr, true);
    QVERIFY(isolated.GetKeybinds("one")->isEmpty());
    settings.Reset();
    QCOMPARE(model.GetCombination(target), 0);
    QVERIFY(settings.GetKeybinds("two")->isEmpty());
  }

  void RejectsMalformedAndDuplicateStoredDocuments() {
    QTemporaryDir directory;
    const auto path = directory.filePath("settings.ini");
    Settings settings(path);
    QCOMPARE(settings.SaveKeybind("account", Range("lamp"), Qt::Key_F6, "Lamp", "Increase"),
             Result::Saved);
    QSettings raw(path, QSettings::IniFormat);
    const auto key = StorageKey("account");
    const auto valid = QJsonDocument::fromJson(raw.value(key).toString().toUtf8()).object();
    QList<QJsonObject> invalid;
    auto document = valid;
    document["version"] = 3;
    invalid.append(document);
    document = valid;
    document["bindings"] = QJsonValue::Null;
    invalid.append(document);
    for (int variant = 0; variant < 5; ++variant) {
      document = valid;
      auto list = document["bindings"].toArray();
      auto binding = list[0].toObject();
      if (variant == 0)
        binding["combination"] = 1.5;
      if (variant == 1)
        binding["action_label"] = QJsonValue::Null;
      if (variant == 2)
        binding["combination"] = int(Qt::Key_Control);
      if (variant == 3) {
        auto target = binding["target"].toObject();
        target["device_id"] = QJsonValue::Null;
        binding["target"] = target;
      }
      if (variant == 4)
        list.append(binding);
      list[0] = binding;
      document["bindings"] = list;
      invalid.append(document);
    }
    for (const auto& document : invalid) {
      const auto serialized =
          QString::fromUtf8(QJsonDocument(document).toJson(QJsonDocument::Compact));
      raw.setValue(key, serialized);
      raw.sync();
      QVERIFY(!settings.GetKeybinds("account"));
      QCOMPARE(settings.SaveKeybind("account", Range("socket"), Qt::Key_F7, "Socket", "Increase"),
               Result::StorageError);
      raw.sync();
      QCOMPARE(raw.value(key).toString(), serialized);
    }
  }

  void RechecksSettingsAtSaveAndReportsOwner() {
    Settings settings(nullptr, true);
    KeybindSettingsModel first(&settings, nullptr), second(&settings, nullptr);
    first.SetAccountId("account");
    second.SetAccountId("account");
    QVERIFY(first.ConflictMessage(Map(Range("lamp")), Qt::Key_F6).isEmpty());
    QVERIFY(second.Save(Map(Range("socket")), Qt::Key_F6, "Reading socket", "Increase brightness"));
    QVERIFY(!first.Save(Map(Range("lamp")), Qt::Key_F6, "Desk lamp", "Increase"));
    QVERIFY(first.GetLastError().contains("Reading socket"));
    QVERIFY(first.GetLastError().contains("Increase brightness"));
    QCOMPARE(first.GetCombination(Map(Range("lamp"))), 0);
    QVERIFY(second.Save(Map(Range("socket")), 0, "Socket", {}));
    QVERIFY(first.ConflictMessage(Map(Range("lamp")), Qt::Key_F6).isEmpty());
    QVERIFY(first.Save(Map(Range("lamp")), Qt::Key_F6, "Lamp", "Increase"));
  }

  void ReportsWriteFailuresWithoutChangingDisplayedAssignment() {
    QTemporaryDir directory;
    const auto path = directory.filePath("settings.ini");
    QVERIFY(QDir().mkdir(path));
    Settings settings(path);
    KeybindSettingsModel model(&settings, nullptr);
    model.SetAccountId("account");
    QVERIFY(!model.Save(Map(Range("lamp")), Qt::Key_F6, "Lamp", "Increase"));
    QVERIFY(!model.GetLastError().isEmpty());
    QCOMPARE(model.GetCombination(Map(Range("lamp"))), 0);
  }
};

QTEST_GUILESS_MAIN(KeybindSettingsTests)
#include "KeybindSettingsTests.moc"
