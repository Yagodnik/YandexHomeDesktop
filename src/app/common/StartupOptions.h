#pragma once

#include <expected>
#include <optional>
#include <QStringList>

struct StartupOptions {
  enum class RestCommand { None, Enable, Disable, Status, Serve };
  bool use_fake_api = false;
  QString fixture_path = ":/debug/api.json";
  RestCommand rest_command = RestCommand::None;
  std::optional<quint16> rest_port;
  QStringList cli_arguments;
};

std::expected<StartupOptions, QString>
ParseStartupOptions(const QStringList& arguments, bool allow_fake_api, bool rest_worker = false);
