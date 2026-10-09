#include "CliApp.h"
#include "app/rest/RestCommand.h"

#include "api/QtHttpTransport.h"
#include "api/YandexAccountApi.h"
#include "api/YandexHomeApi.h"
#include "auth/AuthorizationFactory.h"
#include "cli/CliProgress.h"
#include "cli/CliRunner.h"
#include "utils/Settings.h"
#include <QElapsedTimer>
#include <QTimer>
#include <cstdio>
#include <memory>
#ifdef Q_OS_WIN
#include <io.h>
#else
#include <unistd.h>
#endif
#ifdef YH_DEBUG_FAKE_API
#include "api/debug/FixtureApi.h"
#endif

namespace {
bool IsTerminal(FILE* stream) {
#ifdef Q_OS_WIN
  return _isatty(_fileno(stream)) != 0;
#else
  return isatty(fileno(stream)) != 0;
#endif
}

void Write(const QByteArray& text, bool error) {
  FILE* stream = error ? stderr : stdout;
  std::fwrite(text.constData(), 1, text.size(), stream);
  std::fflush(stream);
}
} // namespace

int RunCli(QCoreApplication& app, const StartupOptions& options) {
  if (options.rest_command != StartupOptions::RestCommand::None) {
    if (options.rest_command == StartupOptions::RestCommand::Serve) {
      Write(CliRunner::FormatError(
                options.cli_arguments.contains("--json"), "usage",
                QCoreApplication::translate("StartupMessages",
                                            "Use YandexHomeRest to run the REST server.")),
            true);
      return CliRunner::Usage;
    }
#ifdef YH_DEBUG_FAKE_API
    if (options.use_fake_api && options.rest_command == StartupOptions::RestCommand::Enable) {
      const auto valid = FixtureApi(options.fixture_path).Validate();
      if (!valid) {
        Write(CliRunner::FormatError(options.cli_arguments.contains("--json"), "fixture_error",
                                     valid.error().message),
              true);
        return CliRunner::RequestFailed;
      }
    }
#endif
    return RunRestControl(app, options);
  }
  const bool json =
      options.cli_arguments.contains("--json") || options.cli_arguments.contains("-j");
  const auto parsed = CliCommand::Parse(options.cli_arguments);
  if (!parsed) {
    Write(CliRunner::FormatError(json, "usage", parsed.error()), true);
    return CliRunner::Usage;
  }
  if (!parsed->operation->UsesServices()) {
    CliRunner runner({}, Write);
    int result = CliRunner::Success;
    QObject::connect(&runner, &CliRunner::finished, &app, [&result](int code) { result = code; });
    runner.Start(*parsed);
    return result;
  }

  std::unique_ptr<IAuthorizationService> auth(CreateAuthorizationService(
      options.use_fake_api ? AuthorizationMode::Fixture : AuthorizationMode::SavedTokenOnly,
      nullptr));
  QtHttpTransport transport(&app, parsed->timeout_ms);
  const auto token_provider = [&auth] { return auth->GetToken().value_or(QString{}); };
  YandexHomeApi live_home(token_provider, &transport, &app);
  YandexAccountApi live_account(token_provider, &transport, &app);
  IHomeApi* home_api = &live_home;
  IAccountApi* account_api = &live_account;
#ifdef YH_DEBUG_FAKE_API
  std::unique_ptr<FixtureApi> fixture;
  if (options.use_fake_api) {
    fixture = std::make_unique<FixtureApi>(options.fixture_path);
    const auto valid = fixture->Validate();
    if (!valid) {
      Write(CliRunner::FormatError(parsed->json, "fixture_error", valid.error().message), true);
      return CliRunner::RequestFailed;
    }
    home_api = fixture.get();
    account_api = fixture.get();
  }
#endif
  HomeService home(home_api);
  DeviceService devices(home_api);
  ScenarioService scenarios(home_api);
  AccountService account(account_api);
  CliProgress progress(CliProgress::EnabledFor(*parsed, IsTerminal(stderr)),
                       [](const QByteArray& text) { Write(text, true); });
  const auto write_result = [&progress](const QByteArray& text, bool error) {
    progress.Stop();
    Write(text, error);
  };
  const auto reset = [&auth, &options](QObject* context, ApiResultHandler<void> handler) {
    auto delivered = std::make_shared<bool>(false);
    QObject::connect(auth.get(), &IAuthorizationService::logoutFinished, context,
                     [handler, delivered] {
                       if (!*delivered) {
                         *delivered = true;
                         handler(ApiResult<void>{});
                       }
                     });
    QObject::connect(auth.get(), &IAuthorizationService::logoutFailed, context,
                     [handler, delivered](const QString& error) {
                       if (!*delivered) {
                         *delivered = true;
                         handler(std::unexpected(ApiError{ApiErrorKind::Service, error}));
                       }
                     });
    auto* rest_settings = new Settings(context, options.use_fake_api);
    auto* rest_control = new RestControlService(rest_settings, RestConfiguration(options), context);
    QObject::connect(
        rest_control, &RestControlService::finished, context,
        [&auth, &options, handler, delivered](const QJsonObject& result) {
          if (!result["ok"].toBool()) {
            *delivered = true;
            handler(std::unexpected(
                ApiError{ApiErrorKind::Service, result["error"].toObject()["message"].toString()}));
            return;
          }
          if (!options.use_fake_api)
            Settings::ResetStoredSettings();
          auth->Logout();
        });
    rest_control->Request(RestControlService::Command::Disable);
  };
  CliRunner runner({&home, &devices, &scenarios, &account, reset}, write_result);
  QObject::connect(&runner, &CliRunner::finished, &app, &QCoreApplication::exit);

  QElapsedTimer elapsed;
  QTimer auth_timeout;
  auth_timeout.setSingleShot(true);
  bool dispatched = false;
  const auto fail_auth = [&](int code, const QString& error_code, const QString& message) {
    if (dispatched) {
      return;
    }
    dispatched = true;
    auth_timeout.stop();
    write_result(CliRunner::FormatError(parsed->json, error_code, message), true);
    app.exit(code);
  };
  const auto dispatch = [&] {
    if (dispatched) {
      return;
    }
    const auto remaining = parsed->timeout_ms - elapsed.elapsed();
    if (remaining <= 0) {
      fail_auth(CliRunner::Timeout, "timeout",
                QCoreApplication::translate("CliApp", "Время выполнения команды истекло."));
      return;
    }
    dispatched = true;
    auth_timeout.stop();
    auto command = *parsed;
    command.timeout_ms = static_cast<int>(remaining);
    progress.SetMessage(QCoreApplication::translate("CliApp", "Выполнение команды..."));
    runner.Start(command);
  };
  QObject::connect(auth.get(), &IAuthorizationService::authorized, &app, dispatch);
  QObject::connect(auth.get(), &IAuthorizationService::unauthorized, &app, [&] {
    fail_auth(CliRunner::Unauthorized, "authorization_required",
              QCoreApplication::translate("CliApp", "Сначала войдите в аккаунт через приложение."));
  });
  QObject::connect(auth.get(), &IAuthorizationService::authorizationFailed, &app, [&] {
    fail_auth(
        CliRunner::Unauthorized, "authorization_failed",
        QCoreApplication::translate("CliApp", "Не удалось прочитать сохранённые данные входа."));
  });
  QObject::connect(auth.get(), &IAuthorizationService::authorizationCanceled, &app, [&] {
    fail_auth(CliRunner::Unauthorized, "authorization_canceled",
              QCoreApplication::translate("CliApp", "Доступ к сохранённым данным входа отменён."));
  });
  QObject::connect(&auth_timeout, &QTimer::timeout, &app, [&] {
    fail_auth(CliRunner::Timeout, "timeout",
              QCoreApplication::translate("CliApp", "Время выполнения команды истекло."));
  });
  // Synchronous API implementations can finish immediately, so dispatch only
  // after the event loop starts. Reset works without a valid saved login.
  QTimer::singleShot(0, &app, [&] {
    elapsed.start();
    auth_timeout.start(parsed->timeout_ms);
    if (!parsed->operation->RequiresAuthorization()) {
      dispatch();
    } else {
      progress.SetMessage(QCoreApplication::translate(
          "CliApp", "Чтение сохранённых данных входа; подтвердите запрос системы..."));
      auth->AttemptLocalAuthorization();
    }
  });
  return app.exec();
}
