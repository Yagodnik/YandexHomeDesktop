#pragma once

#include <QCoreApplication>
#include <QMap>
#include <expected>

struct CliArguments {
  Q_DECLARE_TR_FUNCTIONS(CliArguments)
public:
  QMap<QString, QString> options;
  [[nodiscard]] bool Has(const QString& option) const { return options.contains(option); }
  [[nodiscard]] QString Value(const QString& option) const { return options.value(option); }
};

struct CliTarget {
  QString id, name, household;
  static std::expected<CliTarget, QString> Parse(const CliArguments& arguments, bool allow_household);
};

class CommandRegistry;
void AddTargetOptions(CommandRegistry& registry);
