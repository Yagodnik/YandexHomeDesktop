#pragma once

#include <expected>
#include <optional>
#include <QStringList>

struct StartupOptions {
  enum class RestCommand { None, Enable, Disable, Status, Serve };
  enum class McpCommand { None, Enable, Disable, Status };
  bool use_fake_api = false;
  bool use_fake_auth = false;
  QString fixture_path = ":/debug/api.json";
  RestCommand rest_command = RestCommand::None;
  McpCommand mcp_command = McpCommand::None;
  std::optional<quint16> mcp_port;
  std::optional<quint16> rest_port;
  QString rest_config_file;
  QStringList cli_arguments;
};

std::expected<StartupOptions, QString> ParseStartupOptions(const QStringList& arguments,
                                                           bool allow_fake_api,
                                                           bool rest_worker = false,
                                                           bool allow_fake_auth = false);
