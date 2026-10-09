#include <QCoreApplication>
#include <QJsonObject>
#include <iostream>
#include "api/QtHttpTransport.h"
#include "api/YandexHomeApi.h"
#include "auth/AuthorizationService.h"
#include "cli/CommandRegistry.h"
#include "iot/core/CapabilityState.h"
#include "services/HomeService.h"
#ifdef YH_EXAMPLE_REST
#include "rest/RestRouter.h"
#include "rest/RestServer.h"
#endif

class ExampleApi final : public IHomeApi {
public:
  void GetUserInfo(QObject*, ApiResultHandler<UserInfo> handler) override {
    UserInfo home{};
    home.status = Status::Ok;
    home.devices = {DeviceObject{.id = "lamp", .name = "SDK example lamp"}};
    handler(home);
  }
  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>> handler) override { handler({}); }
  void GetDeviceInfo(const QString&, QObject*, ApiResultHandler<DeviceInfo> handler) override { handler(DeviceInfo{}); }
  void ExecuteScenario(const QString&, QObject*, ApiResultHandler<void> handler) override { handler({}); }
  void PerformActions(const QList<DeviceActionsObject>&, QObject*, ApiResultHandler<void> handler) override { handler({}); }
};

class ExampleTokenStore final : public ITokenStore {
public:
  void Read(QObject*, AuthResultHandler<QString> handler) override {
    handler(std::unexpected(AuthError{AuthErrorKind::NotFound}));
  }
  void Write(const QString&, QObject*, AuthResultHandler<void> handler) override { handler({}); }
  void Delete(QObject*, AuthResultHandler<void> handler) override { handler({}); }
};

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  ExampleApi api;
  HomeService home(&api);
  home.EnsureLoaded();
  if (home.GetLoadState() != HomeService::LoadState::Ready || home.GetSnapshot().devices.size() != 1) return 1;
  const auto decoded = Serialization::From<UserInfo>(QJsonObject{{"status", "ok"}});
  if (decoded.status != Status::Ok || !Iot::State::OnOff(true)["value"].toBool()) return 2;
  ExampleTokenStore store;
  AuthorizationService auth(&store);
  if (auth.IsAuthorized()) return 3;
  // Construction is lazy: no HTTP, OAuth, or credential-store access.
  QtHttpTransport transport;
  YandexHomeApi live([] { return QString{}; }, &transport);
  if (!CommandRegistry::Builtin().Help("example").contains("devices set")) return 4;
#ifdef YH_EXAMPLE_REST
  RestRouter routes;
  RestServer server(routes);
  if (server.Port() != 0) return 5;
#endif
  std::cout << home.GetSnapshot().devices.first().name.toStdString() << '\n';
  return 0;
}
