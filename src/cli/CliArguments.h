#pragma once

#include "yh/appcli_export.h"

#include <QCoreApplication>
#include <QMap>
#include <expected>

struct CliArguments {
  Q_DECLARE_TR_FUNCTIONS(CliArguments)
public:
  QMap<QString, QString> options;
  [[nodiscard]] bool Has(const QString& option) const {
    return options.contains(option);
  }
  [[nodiscard]] QString Value(const QString& option) const {
    return options.value(option);
  }
};

struct APPCLI_EXPORT CliTarget {
  QString id, name, household;
  static std::expected<CliTarget, QString> Parse(const CliArguments& arguments,
                                                 bool allow_household);
};

class CommandRegistry;
APPCLI_EXPORT void AddTargetOptions(CommandRegistry& registry);
