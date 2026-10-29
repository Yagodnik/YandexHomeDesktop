#include "McpHttpServer.h"

#include <QHttpServerRequest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPointer>
#include <QTimer>
#include <memory>
#include "rest/RestReply.h"
#include "rest/RestRouter.h"
#include "rest/RestServer.h"
#include "worker/WorkerControlClient.h"

namespace {
QJsonObject RpcError(const QJsonValue& id, int code, const QString& message) {
  return {{"jsonrpc", "2.0"}, {"id", id},
          {"error", QJsonObject{{"code", code}, {"message", message}}}};
}
void Result(RestReply* reply, const QJsonValue& id, const QJsonObject& result) {
  reply->SendDocument({{"jsonrpc", "2.0"}, {"id", id}, {"result", result}});
}

void Forward(RestReply* reply, const QString& endpoint, const QJsonValue& id,
             const QJsonObject& request, bool tools) {
  auto* client = new WorkerControlClient({endpoint, {}, {}, "stop"}, reply);
  auto stage = std::make_shared<int>(0);
  auto request_id = std::make_shared<QString>();
  QObject::connect(client, &WorkerControlClient::finished, reply,
      [reply, client, stage, request_id, id, tools](const QJsonObject& response) {
        if (!response.value("ok").toBool()) {
          const auto error = response.value("error");
          const auto message = error.isString() ? error.toString() : error.toObject().value("code").toString();
          reply->SendDocument(RpcError(id, -32000, message.isEmpty() ? QStringLiteral("GUI unavailable") : message));
          return;
        }
        if (tools) { Result(reply, id, {{"tools", response.value("tools").toArray()}}); return; }
        if (*stage == 0) {
          *request_id = response.value("id").toString();
          if (request_id->isEmpty()) { reply->SendDocument(RpcError(id, -32000, "Invalid GUI response")); return; }
          *stage = 1;
        } else if (!response.value("pending").toBool()) {
          const auto result = response.value("result").toObject();
          const auto text = QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
          Result(reply, id, {{"content", QJsonArray{QJsonObject{{"type", "text"}, {"text", text}}}},
                             {"isError", !result.value("ok").toBool()}});
          return;
        }
        QTimer::singleShot(50, reply, [client, request_id] {
          client->Request(WorkerControlClient::Command::Send, {{"command", "poll"}, {"id", *request_id}});
        });
      });
  client->Request(WorkerControlClient::Command::Send, request);
}

void Handle(const QHttpServerRequest& request, RestReply* reply, const QString& endpoint) {
  QJsonParseError parse_error;
  const auto document = QJsonDocument::fromJson(request.body(), &parse_error);
  if (parse_error.error != QJsonParseError::NoError) {
    reply->SendDocument(RpcError(QJsonValue::Null, -32700, "Parse error")); return;
  }
  const auto message = document.object();
  if (!document.isObject() || message.value("jsonrpc") != "2.0" || !message.value("method").isString()) {
    reply->SendDocument(RpcError(QJsonValue::Null, -32600, "Invalid Request")); return;
  }
  if (!message.contains("id")) { reply->SendEmpty(RestReply::StatusCode::Accepted); return; }
  const auto id = message.value("id");
  if (!id.isString() && !id.isDouble()) {
    reply->SendDocument(RpcError(QJsonValue::Null, -32600, "Invalid request ID")); return;
  }
  const auto method = message.value("method").toString();
  if (method == "initialize") {
    Result(reply, id, {{"protocolVersion", "2025-11-25"}, {"capabilities", QJsonObject{{"tools", QJsonObject{}}}},
                       {"serverInfo", QJsonObject{{"name", "yandex-home"}, {"version", "0.1.0"}}},
                       {"instructions", "Resolve names and rooms from list_devices; ask when there is more than one match."}});
  } else if (method == "ping") {
    Result(reply, id, {});
  } else if (method == "tools/list") {
    Forward(reply, endpoint, id, {{"command", "tools"}}, true);
  } else if (method == "tools/call") {
    const auto params = message.value("params").toObject();
    if (!message.value("params").isObject() || !params.value("name").isString() ||
        (params.contains("arguments") && !params.value("arguments").isObject())) {
      reply->SendDocument(RpcError(id, -32602, "Invalid parameters")); return;
    }
    Forward(reply, endpoint, id, {{"command", "submit"}, {"tool", params.value("name")},
                                  {"arguments", params.value("arguments").toObject()}}, false);
  } else {
    reply->SendDocument(RpcError(id, -32601, "Method not found"));
  }
}
} // namespace

McpHttpServer::McpHttpServer(QString gui_endpoint, QByteArray token, QObject* parent) : QObject(parent) {
  routes_ = std::make_unique<RestRouter>();
  routes_->Add(QHttpServerRequest::Method::Post, "/mcp",
             [endpoint = std::move(gui_endpoint), token = std::move(token)](const auto& request, const auto&, RestReply* reply) {
               if (request.headers().value("Authorization").toByteArray() != "Bearer " + token) {
                 reply->Fail(RestReply::StatusCode::Unauthorized, "unauthorized", "A valid bearer token is required.");
                 return;
               }
               Handle(request, reply, endpoint);
             });
  server_ = std::make_unique<RestServer>(*routes_, nullptr, 30'000);
}
McpHttpServer::~McpHttpServer() = default;
bool McpHttpServer::Start(quint16 port) { return server_->Start(port); }
void McpHttpServer::Stop() { server_->Stop(); }
quint16 McpHttpServer::Port() const { return server_->Port(); }
QString McpHttpServer::Error() const { return server_->Error(); }
