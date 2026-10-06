#include "CliApp.h"

#include <QElapsedTimer>
#include <QPointer>
#include <QTimer>
#include <cstdio>
#include <memory>
#include "auth/AuthorizationService.h"
#include "api/QtHttpTransport.h"
#include "api/YandexHomeApi.h"
#include "api/YandexAccountApi.h"
#include "cli/CliRunner.h"
#include "utils/Settings.h"
#ifdef YH_DEBUG_FAKE_API
#include "api/debug/FixtureApi.h"
#endif

namespace {
void Write(const QByteArray& text, bool error) {
  FILE* stream = error ? stderr : stdout;
  std::fwrite(text.constData(), 1, text.size(), stream);
  std::fflush(stream);
}
}

int RunCli(QCoreApplication& app, const StartupOptions& options) {
  const bool json = options.cli_arguments.contains("--json") || options.cli_arguments.contains("-j");
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

  AuthorizationService auth(&app, options.use_fake_api, false);
  QtHttpTransport transport(&app, parsed->timeout_ms);
  const auto token_provider = [&auth] { return auth.GetToken().value_or(QString{}); };
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
  CliRunner runner({&home, &devices, &scenarios, &account,
    [&auth, &options](QObject* context, ApiResultHandler<void> handler) {
      auto delivered = std::make_shared<bool>(false);
      QObject::connect(&auth, &AuthorizationService::logoutFinished, context, [handler, delivered] {
        if (!*delivered) { *delivered = true; handler(ApiResult<void>{}); }
      });
      QObject::connect(&auth, &AuthorizationService::logoutFailed, context, [handler, delivered](const QString& error) {
        if (!*delivered) { *delivered = true; handler(std::unexpected(ApiError{ApiErrorKind::Service, error})); }
      });
      if (!options.use_fake_api) { Settings::ResetStoredSettings(); }
      auth.Logout();
    }}, Write);
  QObject::connect(&runner, &CliRunner::finished, &app, &QCoreApplication::exit);

  QElapsedTimer elapsed;
  QTimer auth_timeout;
  auth_timeout.setSingleShot(true);
  bool dispatched = false;
  const auto fail_auth = [&](int code, const QString& error_code, const QString& message) {
    if (dispatched) { return; }
    dispatched = true;
    auth_timeout.stop();
    Write(CliRunner::FormatError(parsed->json, error_code, message), true);
    app.exit(code);
  };
  const auto dispatch = [&] {
    if (dispatched) { return; }
    const auto remaining = parsed->timeout_ms - elapsed.elapsed();
    if (remaining <= 0) {
      fail_auth(CliRunner::Timeout, "timeout", QCoreApplication::translate("CliApp", "Время выполнения команды истекло."));
      return;
    }
    dispatched = true;
    auth_timeout.stop();
    auto command = *parsed;
    command.timeout_ms = static_cast<int>(remaining);
    runner.Start(command);
  };
  QObject::connect(&auth, &AuthorizationService::authorized, &app, dispatch);
  QObject::connect(&auth, &AuthorizationService::unauthorized, &app, [&] {
    fail_auth(CliRunner::Unauthorized, "authorization_required",
      QCoreApplication::translate("CliApp", "Сначала войдите в аккаунт через приложение."));
  });
  QObject::connect(&auth, &AuthorizationService::authorizationFailed, &app, [&] {
    fail_auth(CliRunner::Unauthorized, "authorization_failed",
      QCoreApplication::translate("CliApp", "Не удалось прочитать сохранённые данные входа."));
  });
  QObject::connect(&auth, &AuthorizationService::authorizationCanceled, &app, [&] {
    fail_auth(CliRunner::Unauthorized, "authorization_canceled",
      QCoreApplication::translate("CliApp", "Доступ к сохранённым данным входа отменён."));
  });
  QObject::connect(&auth_timeout, &QTimer::timeout, &app, [&] {
    fail_auth(CliRunner::Timeout, "timeout", QCoreApplication::translate("CliApp", "Время выполнения команды истекло."));
  });
  // Synchronous API implementations can finish immediately, so dispatch only
  // after the event loop starts. Reset works without a valid saved login.
  QTimer::singleShot(0, &app, [&] {
    elapsed.start();
    auth_timeout.start(parsed->timeout_ms);
    if (!parsed->operation->RequiresAuthorization()) { dispatch(); }
    else { auth.AttemptLocalAuthorization(); }
  });
  return app.exec();
}
