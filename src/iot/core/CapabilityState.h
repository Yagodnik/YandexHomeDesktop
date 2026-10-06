#pragma once

#include <QVariantMap>

namespace Iot::State {
QVariantMap Boolean(const QString& instance, bool value);
QVariantMap OnOff(bool value);
QVariantMap WithRelative(QVariantMap state);
QVariantMap Toggle(const QString& instance, bool value);
QVariantMap Range(const QString& instance, double value);
QVariantMap RelativeRange(const QString& instance, double delta);
QVariantMap Mode(const QString& instance, const QString& value);
QVariantMap Color(const QString& instance, const QVariant& value);
QVariantMap Rgb(const QVariant& value);
QVariantMap Hsv(const QVariantMap& value);
QVariantMap Temperature(const QVariant& value);
QVariantMap Scene(const QString& value);
quint32 PackRgb(int red, int green, int blue);
QVariantMap HsvComponents(int hue, int saturation, int value);
}
