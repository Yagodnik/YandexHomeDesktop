#include "ValueParsers.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QRegularExpression>

namespace CliCapability {
ValueResult BooleanValue(const QString& text) {
  static const QMap<QString, bool> values{
      {"on", true}, {"true", true}, {"off", false}, {"false", false}};
  const auto found = values.constFind(text);
  if (found == values.cend()) {
    return std::unexpected(
        CliValueParsers::tr("Логическое значение должно быть on/off или true/false."));
  }
  return QVariant(*found);
}
ValueResult NumberValue(const QString& text) {
  bool valid = false;
  const auto number = text.toDouble(&valid);
  if (!valid) {
    return std::unexpected(CliValueParsers::tr("Значение должно быть конечным числом."));
  }
  return QVariant(number);
}
ValueResult RgbValue(const QString& text) {
  static const QRegularExpression hex("^(#[0-9a-fA-F]{6}|0x[0-9a-fA-F]{6})$");
  if (hex.match(text).hasMatch()) {
    const auto digits = text.mid(text.startsWith('#') ? 1 : 2);
    return QVariant(double(digits.toUInt(nullptr, 16)));
  }
  return NumberValue(text);
}
ValueResult HsvValue(const QString& text) {
  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(text.toUtf8(), &error);
  if (error.error != QJsonParseError::NoError || !document.isObject()) {
    return std::unexpected(CliValueParsers::tr("HSV должен быть JSON-объектом с h, s и v."));
  }
  return QVariant(document.object().toVariantMap());
}
ValueResult TextValue(const QString& text) {
  return QVariant(text);
}
} // namespace CliCapability
