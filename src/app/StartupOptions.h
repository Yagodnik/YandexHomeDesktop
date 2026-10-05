#pragma once

#include <expected>
#include <QStringList>

struct StartupOptions {
  bool use_fake_api = false;
  QString fixture_path = ":/debug/api.json";
  QStringList cli_arguments;
};

std::expected<StartupOptions, QString> ParseStartupOptions(const QStringList& arguments, bool allow_fake_api);
