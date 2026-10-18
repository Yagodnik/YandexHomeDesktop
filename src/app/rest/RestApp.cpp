#include "RestApp.h"
#include <QTimer>
#include "RestCommand.h"
#include "RestServerConfiguration.h"
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
#include "worker/WorkerControlServer.h"
#ifdef YH_DEBUG_FAKE_API
#include "api/debug/FixtureApi.h"
#endif
namespace {
constexpr int ShutdownDelayMs = 100;
constexpr int StartupErrorGraceMs = 1'000;
int Serve(QCoreApplication& app, const StartupOptions& options, Settings& settings,
          const RestLaunchConfiguration& configuration,
          const RestServerConfiguration& server_configuration, bool json, int timeout_ms) {
  WorkerControlServer control;
  if (!control.Start(configuration.control_name)) {
    const auto code = control.ErrorCode();
    RestPrint(RestError(json, code,
                        code == "already_running"
                            ? RestAppMessages::tr("Сервер REST API уже запущен.")
                            : RestAppMessages::tr("Не удалось открыть канал управления REST API.")),
              true);
    return RestConsole::RequestFailed;
  }
  std::unique_ptr<IAuthorizationService> auth;
  if (options.use_fake_api || server_configuration.oauth_token.isEmpty()) {
    auth.reset(CreateAuthorizationService(options.use_fake_api ? AuthorizationMode::Fixture
                                                               : AuthorizationMode::SavedTokenOnly,
                                          nullptr));
  }
  QtHttpTransport transport(&app, timeout_ms);
  const auto token_provider = [&auth, &server_configuration] {
    return auth ? auth->GetToken().value_or(QString{}) : server_configuration.oauth_token;
  };
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
  const auto port =
      options.rest_port.value_or(server_configuration.port.value_or(settings.GetRestPort()));
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
                       {"host", server_configuration.host.toString()},
                       {"port", port},
                       {"url", server_configuration.Url(port)}};
  };
  control.SetHandler([&](const QJsonObject& request) -> QJsonObject {
    const auto command = request["command"].toString();
    if (command == "disable") {
      starting = false;
      running = false;
      rest.Stop();
      settings.SetRestEnabled(false);
      startup_error = {};
      QTimer::singleShot(ShutdownDelayMs, &app, &QCoreApplication::quit);
      return status();
    }
    if (command == "status")
      return status();
    return {{"ok", false},
            {"error", QJsonObject{{"code", "disabled"},
                                  {"message", RestAppMessages::tr("REST API отключён.")}}}};
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
    if (!rest.Start(port, server_configuration.host)) {
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
  if (auth) {
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
  }
  QObject::connect(&startup_timeout, &QTimer::timeout, &app, [&] {
    fail("timeout", RestAppMessages::tr("Время запуска REST API истекло."));
  });
  QTimer::singleShot(0, &app, [&] {
    startup_timeout.start(timeout_ms);
    if (auth) {
      auth->AttemptLocalAuthorization();
    } else {
      start();
    }
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
  const auto server_configuration = LoadRestServerConfiguration(options.rest_config_file);
  if (!server_configuration) {
    RestPrint(RestError(parsed->json, "config_error", server_configuration.error()), true);
    return RestConsole::Usage;
  }
  Settings settings(nullptr, options.use_fake_api || !options.rest_config_file.isEmpty());
  return Serve(app, options, settings, RestConfiguration(options), *server_configuration,
               parsed->json, parsed->timeout_ms);
}
