#pragma once

#include "yh/appcli_export.h"

#include <QCoreApplication>
#include <QVariant>
#include <expected>

class CliValueParsers {
  Q_DECLARE_TR_FUNCTIONS(CliValueParsers)
};
namespace CliCapability {
using ValueResult = std::expected<QVariant, QString>;
using ValueParser = ValueResult (*)(const QString&);
APPCLI_EXPORT ValueResult BooleanValue(const QString& text);
APPCLI_EXPORT ValueResult NumberValue(const QString& text);
APPCLI_EXPORT ValueResult RgbValue(const QString& text);
APPCLI_EXPORT ValueResult HsvValue(const QString& text);
APPCLI_EXPORT ValueResult TextValue(const QString& text);
} // namespace CliCapability
