#include "ShortcutConfiguration.h"
#include "QHotkeyRegistrar.h"
#include "app/common/AppTranslations.h"
#include "auth/AuthorizationFactory.h"
#include "api/QtHttpTransport.h"
#include "api/YandexHomeApi.h"
#include "api/YandexAccountApi.h"
#include "worker/WorkerControlServer.h"
#include "services/ShortcutRuntime.h"
#include <QGuiApplication>
#include <QJsonDocument>
#include <iostream>
#ifdef YH_DEBUG_FAKE_API
#include "api/debug/FixtureApi.h"
#endif

namespace {
void Print(const QJsonObject& result) {
  std::cout << QJsonDocument(result).toJson(QJsonDocument::Compact).constData() << std::endl;
}
QJsonObject Error(const QString& code) {
  return {{"ok", false}, {"error", QJsonObject{{"code", code}}}};
}
#ifdef YH_DEBUG_FAKE_API
// Fixture runs never claim native shortcuts or use the real account/token store.
class FixtureRegistrar final : public IShortcutRegistrar {
public:
  QHash<int, std::function<void()>> callbacks;
  bool Register(int combination, std::function<void()> activated) override {
    callbacks.insert(combination, std::move(activated));
    return true;
  }
  bool Clear() override {
    callbacks.clear();
    return true;
  }
};
#endif
} // namespace

int main(int argc, char* argv[]) {
  QStringList arguments;
  for (int i = 0; i < argc; ++i)
    arguments.append(QString::fromLocal8Bit(argv[i]));
#ifdef YH_DEBUG_FAKE_API
  constexpr bool allow_fake_api = true;
#else
  constexpr bool allow_fake_api = false;
#endif
  const auto options = ParseStartupOptions(arguments, allow_fake_api);
  const bool help = arguments.contains("--help") || arguments.contains("-h");
  const bool status_command = arguments.contains("--status");
  const bool stop_command = arguments.contains("--stop");
  bool valid = options && options->rest_command == StartupOptions::RestCommand::None &&
               !(status_command && stop_command);
  if (options)
    for (const auto& argument : options->cli_arguments.mid(1))
      valid &= argument == "--help" || argument == "-h" || argument == "--json" ||
               argument == "--status" || argument == "--stop";
  if (!valid || help || status_command || stop_command) {
    QCoreApplication app(argc, argv);
    if (!valid) {
      Print(Error("usage"));
      return 2;
    }
    if (help) {
      Print({{"ok", true}, {"usage", "YandexHomeShortcuts [--status | --stop] [--json]"}});
      return 0;
    }
    WorkerControlClient control(ShortcutConfiguration(*options));
    QObject::connect(&control, &WorkerControlClient::finished, &app,
                     [&app](const QJsonObject& result) {
                       Print(result);
                       app.exit(result["ok"].toBool() ? 0 : 1);
                     });
    QTimer::singleShot(0, &app, [&] {
      control.Request(stop_command ? WorkerControlClient::Command::Stop
                                   : WorkerControlClient::Command::Status);
    });
    return app.exec();
  }
  QGuiApplication app(argc, argv);
  app.setApplicationName("YandexHomeShortcuts");
  app.setQuitOnLastWindowClosed(false);
#ifdef Q_OS_MACOS
  extern void HideShortcutWorker();
  HideShortcutWorker();
#endif
  WorkerControlServer control;
  if (!control.Start(ShortcutConfiguration(*options).control_name)) {
    Print(Error(control.ErrorCode()));
    return 1;
  }
  std::unique_ptr<IAuthorizationService> auth(CreateAuthorizationService(
      options->use_fake_api ? AuthorizationMode::Fixture : AuthorizationMode::SavedTokenOnly,
      nullptr));
  QtHttpTransport transport(&app);
  const auto token = [&auth] { return auth->GetToken().value_or(QString{}); };
  YandexHomeApi live_home(token, &transport);
  YandexAccountApi live_account(token, &transport);
  IHomeApi* home = &live_home;
  IAccountApi* account = &live_account;
  QHotkeyRegistrar native_registrar;
  IShortcutRegistrar* registrar = &native_registrar;
#ifdef YH_DEBUG_FAKE_API
  std::unique_ptr<FixtureApi> fixture;
  FixtureRegistrar fixture_registrar;
  if (options->use_fake_api) {
    fixture = std::make_unique<FixtureApi>(options->fixture_path);
    if (!fixture->Validate()) {
      Print(Error("fixture_error"));
      return 1;
    }
    home = fixture.get();
    account = fixture.get();
    registrar = &fixture_registrar;
  }
#endif
  DeviceService devices(home);
  ScenarioService scenarios(home);
  ShortcutActionService actions(&devices, &scenarios, nullptr);
  std::unique_ptr<ShortcutRuntime> runtime;
  QJsonObject startup_error;
  QTimer deadline;
  deadline.setSingleShot(true);
  bool stopping = false;
  const auto stop = [&] {
    stopping = true;
    deadline.stop();
    if (runtime)
      runtime->Stop();
    registrar->Clear();
    actions.Reset();
    QTimer::singleShot(100, &app, &QCoreApplication::quit);
  };
  const auto fail = [&](const QString& code) {
    if (stopping || !startup_error.isEmpty())
      return;
    startup_error = Error(code);
    deadline.stop();
    QTimer::singleShot(1'000, &app, [&app] { app.exit(1); });
  };
  control.SetHandler([&](const QJsonObject& request) -> QJsonObject {
    const auto command = request["command"].toString();
    if (command == "stop") {
      stop();
      return {{"ok", true}, {"running", false}};
    }
    if (!startup_error.isEmpty())
      return startup_error;
    if (runtime) {
#ifdef YH_DEBUG_FAKE_API
      if (options->use_fake_api && command == "activate") {
        const auto callback = fixture_registrar.callbacks.value(request["combination"].toInt());
        if (callback)
          callback();
        return runtime->Status();
      }
#endif
      return runtime->Handle(request);
    }
    return {{"ok", true}, {"running", false}, {"starting", !stopping}};
  });
  QObject::connect(auth.get(), &IAuthorizationService::authorized, &app, [&] {
    account->LoadData(&app, [&](ApiResult<AccountInfo> info) {
      if (stopping || !startup_error.isEmpty())
        return;
      if (!info || info->id.isEmpty()) {
        fail("account_error");
        return;
      }
      deadline.stop();
      runtime = std::make_unique<ShortcutRuntime>(registrar, &actions, info->id);
      QObject::connect(runtime.get(), &ShortcutRuntime::stopRequested, &app, stop);
    });
  });
  QObject::connect(auth.get(), &IAuthorizationService::unauthorized, &app,
                   [&] { fail("authorization_required"); });
  QObject::connect(auth.get(), &IAuthorizationService::authorizationFailed, &app,
                   [&] { fail("authorization_failed"); });
  QObject::connect(auth.get(), &IAuthorizationService::authorizationCanceled, &app,
                   [&] { fail("authorization_canceled"); });
  QObject::connect(&deadline, &QTimer::timeout, &app, [&] { fail("timeout"); });
  QTimer::singleShot(0, &app, [&] {
    deadline.start(30'000);
    auth->AttemptLocalAuthorization();
  });
  const auto result = app.exec();
  if (runtime)
    runtime->Stop();
  registrar->Clear();
  return result;
}
