#pragma once

#include <QCommandLineParser>
#include <functional>
#include <memory>
#include "CliArguments.h"
#include "ICommand.h"

class CommandRegistry {
  Q_DECLARE_TR_FUNCTIONS(CommandRegistry)
public:
  using Result = std::expected<std::shared_ptr<const ICommand>, QString>;
  using Factory = std::function<Result(const CliArguments&)>;
  struct Definition {
    QStringList path;
    QString description;
    QStringList options;
    Factory factory;
  };
  struct LegacyAlias {
    QString option;
    QStringList path;
    QString value_option;
    QMap<QString, QString> defaults;
    QStringList forbidden;
    QString help_option;
  };
  void AddOption(const QCommandLineOption& option);
  void Register(Definition definition);
  void RegisterLegacy(LegacyAlias alias);
  void Configure(QCommandLineParser& parser) const;
  QString CanonicalOption(const QString& name) const;
  QString Describe() const;
  QString Help(const QString& program) const;
  Result Create(QStringList path, CliArguments arguments) const;
  static CommandRegistry Builtin();
private:
  struct ResolvedCommand {
    QStringList path;
    CliArguments arguments;
    bool help = false;
  };
  std::expected<ResolvedCommand, QString> ResolveLegacy(QStringList path, CliArguments arguments) const;
  const Definition* Find(const QStringList& path) const;
  static std::expected<void, QString> ValidateOptions(const Definition& command, const CliArguments& arguments);
  QList<QCommandLineOption> options_;
  QList<Definition> commands_;
  QList<LegacyAlias> legacy_;
};
