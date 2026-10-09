#include "RestApp.h"
#include "RestCommand.h"
#include "rest/RestServer.h"
#include "rest/RestEndpoints.h"
#include "services/HomeService.h"
#include "services/DeviceService.h"
#include "services/ScenarioService.h"
#include "services/AccountService.h"
#include "api/QtHttpTransport.h"
#include "api/YandexHomeApi.h"
#include "api/YandexAccountApi.h"
#include "auth/AuthorizationFactory.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLockFile>
#include <QStandardPaths>
#ifdef YH_DEBUG_FAKE_API
#include "api/debug/FixtureApi.h"
#endif
namespace {
constexpr int ControlReadTimeoutMs = 5'000;
constexpr qsizetype MaxControlRequestBytes = 1024;
constexpr int ShutdownDelayMs = 100;
constexpr int StartupErrorGraceMs = 1'000;
int Serve(QCoreApplication& app, const StartupOptions& options, Settings& settings,
          const RestLaunchConfiguration& configuration, bool json, int timeout_ms) {
  QString lock_path;
#ifdef Q_OS_UNIX
  const auto directory = QFileInfo(configuration.control_name).absolutePath();
  const auto permissions = QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner;
  if (!QDir().mkpath(directory) || !QFile::setPermissions(directory, permissions)) {
    RestPrint(RestError(json, "control_error",
                        RestAppMessages::tr("Не удалось открыть канал управления REST API.")),
              true);
    return RestConsole::RequestFailed;
  }
  lock_path = configuration.control_name + ".lock";
#else
  lock_path = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                  .filePath(configuration.control_name + ".lock");
#endif
  QLockFile lock(lock_path);
  // Long-running ownership is based on PID/liveness, never the age of the file.
  lock.setStaleLockTime(0);
  if (!lock.tryLock()) {
    RestPrint(
        RestError(json, "already_running", RestAppMessages::tr("Сервер REST API уже запущен.")),
        true);
    return RestConsole::RequestFailed;
  }
  QLocalServer control;
  control.setSocketOptions(QLocalServer::UserAccessOption);
  QLocalServer::removeServer(configuration.control_name);
  if (!control.listen(configuration.control_name)) {
    RestPrint(RestError(json, "control_error",
                        RestAppMessages::tr("Не удалось открыть канал управления REST API.")),
              true);
    return RestConsole::RequestFailed;
  }
  auto auth = std::unique_ptr<IAuthorizationService>(CreateAuthorizationService(
      options.use_fake_api ? AuthorizationMode::Fixture : AuthorizationMode::SavedTokenOnly,
      nullptr));
  QtHttpTransport transport(&app, timeout_ms);
  const auto token_provider = [&auth] { return auth->GetToken().value_or(QString{}); };
  YandexHomeApi live_home(token_provider, &transport);
  YandexAccountApi live_account(token_provider, &transport);
  IHomeApi* home_api = &live_home;
  IAccountApi* account_api = &live_account;
#ifdef YH_DEBUG_FAKE_API
  std::unique_ptr<FixtureApi> fixture;
  if (options.use_fake_api) {
    fixture = std::make_unique<FixtureApi>(options.fixture_path);
    home_api = fixture.get();
    account_api = fixture.get();
  }
#endif
  HomeService home(home_api);
  DeviceService devices(home_api);
  ScenarioService scenarios(home_api);
  AccountService account(account_api);
  const auto routes = CreateRestRoutes(home, devices, scenarios, account);
  RestServer rest(routes, nullptr, timeout_ms);
  const auto port = options.rest_port.value_or(settings.GetRestPort());
  bool running = false;
  bool starting = true;
  QJsonObject startup_error;
  const auto status = [&] {
    if (!startup_error.isEmpty()) {
      return startup_error;
    }
    return QJsonObject{{"ok", true},
                       {"enabled", running},
                       {"running", running},
                       {"starting", starting},
                       {"pid", QCoreApplication::applicationPid()},
                       {"fixture", options.use_fake_api},
                       {"port", port},
                       {"url", QString("http://127.0.0.1:%1/v1").arg(port)}};
  };
  QObject::connect(&control, &QLocalServer::newConnection, &app, [&] {
    while (auto* socket = control.nextPendingConnection()) {
      auto buffer = std::make_shared<QByteArray>();
      auto handled = std::make_shared<bool>(false);
      auto* deadline = new QTimer(socket);
      deadline->setSingleShot(true);
      QObject::connect(deadline, &QTimer::timeout, socket, &QLocalSocket::abort);
      deadline->start(ControlReadTimeoutMs);
      QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
      QObject::connect(socket, &QLocalSocket::readyRead, socket, [&, socket, buffer, handled] {
        if (*handled) {
          return;
        }
        *buffer += socket->readAll();
        if (buffer->size() > MaxControlRequestBytes) {
          socket->abort();
          return;
        }
        const auto end = buffer->indexOf('\n');
        if (end < 0) {
          return;
        }
        *handled = true;
        const auto command =
            QJsonDocument::fromJson(buffer->left(end)).object()["command"].toString();
        QJsonObject response;
        if (command == "disable") {
          starting = false;
          running = false;
          rest.Stop();
          settings.SetRestEnabled(false);
          startup_error = {};
          response = status();
          QTimer::singleShot(ShutdownDelayMs, &app, &QCoreApplication::quit);
        } else if (command == "status") {
          response = status();
        } else {
          response = {
              {"ok", false},
              {"error", QJsonObject{{"code", "disabled"},
                                    {"message", RestAppMessages::tr("REST API отключён.")}}}};
        }
        socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + '\n');
        socket->flush();
        socket->disconnectFromServer();
      });
    }
  });
  QTimer startup_timeout;
  startup_timeout.setSingleShot(true);
  const auto fail = [&](const QString& code, const QString& message) {
    if (!starting) {
      return;
    }
    starting = false;
    startup_timeout.stop();
    startup_error = {{"ok", false}, {"error", QJsonObject{{"code", code}, {"message", message}}}};
    RestPrint(RestError(json, code, message), true);
    // Keep the local channel alive briefly so the launching command sees the reason.
    QTimer::singleShot(StartupErrorGraceMs, &app, [&app] { app.exit(RestConsole::RequestFailed); });
  };
  const auto start = [&] {
    if (!starting) {
      return;
    }
    if (!rest.Start(port)) {
      fail("listen_error",
           RestAppMessages::tr("Не удалось открыть порт REST API: %1.").arg(rest.Error()));
      return;
    }
    starting = false;
    running = true;
    startup_timeout.stop();
    settings.SetRestPort(port);
    settings.SetRestEnabled(true);
    RestOutput(status(), json);
  };
  QObject::connect(auth.get(), &IAuthorizationService::authorized, &app, start);
  QObject::connect(auth.get(), &IAuthorizationService::unauthorized, &app, [&] {
    fail("authorization_required",
         RestAppMessages::tr("Сначала войдите в аккаунт через приложение."));
  });
  QObject::connect(auth.get(), &IAuthorizationService::authorizationFailed, &app, [&] {
    fail("authorization_failed",
         RestAppMessages::tr("Не удалось прочитать сохранённые данные входа."));
  });
  QObject::connect(auth.get(), &IAuthorizationService::authorizationCanceled, &app, [&] {
    fail("authorization_canceled",
         RestAppMessages::tr("Доступ к сохранённым данным входа отменён."));
  });
  QObject::connect(&startup_timeout, &QTimer::timeout, &app, [&] {
    fail("timeout", RestAppMessages::tr("Время запуска REST API истекло."));
  });
  QTimer::singleShot(0, &app, [&] {
    startup_timeout.start(timeout_ms);
    auth->AttemptLocalAuthorization();
  });
  const auto result = app.exec();
  rest.Stop();
  return result;
}
} // namespace
int RunRest(QCoreApplication& app, const StartupOptions& options) {
  const auto parsed = ParseRestConsole(options);
  if (!parsed)
    return parsed.error();
#ifdef YH_DEBUG_FAKE_API
  if (options.use_fake_api && (options.rest_command == StartupOptions::RestCommand::Enable ||
                               options.rest_command == StartupOptions::RestCommand::Serve)) {
    const auto valid = FixtureApi(options.fixture_path).Validate();
    if (!valid) {
      RestPrint(RestError(parsed->json, "fixture_error", valid.error().message), true);
      return RestConsole::RequestFailed;
    }
  }
#endif
  if (options.rest_command != StartupOptions::RestCommand::Serve)
    return RunRestControl(app, options);
  Settings settings(nullptr, options.use_fake_api);
  return Serve(app, options, settings, RestConfiguration(options), parsed->json,
               parsed->timeout_ms);
}
