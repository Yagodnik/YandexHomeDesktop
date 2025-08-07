#pragma once

#include <QGuiApplication>
#include <QString>

#include "app/AppContext.h"

struct CommandContext {
  bool pro_mode;
};

class ICommand : public QObject {
  Q_OBJECT
public:
  explicit ICommand(const QString& command_name, QObject *parent)
    : command_name_(command_name), QObject(parent) {}

  virtual ~ICommand() = default;

  virtual void Execute(AppContext& app_ctx, const CommandContext& command_ctx) = 0;

protected:
  QString command_name_;
};
