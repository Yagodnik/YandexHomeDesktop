#pragma once

#include <QCoreApplication>
#include <QVariant>
#include <expected>

class CliValueParsers {
  Q_DECLARE_TR_FUNCTIONS(CliValueParsers)
};
namespace CliCapability {
using ValueResult = std::expected<QVariant, QString>;
using ValueParser = ValueResult (*)(const QString&);
ValueResult BooleanValue(const QString& text);
ValueResult NumberValue(const QString& text);
ValueResult RgbValue(const QString& text);
ValueResult HsvValue(const QString& text);
ValueResult TextValue(const QString& text);
} // namespace CliCapability
