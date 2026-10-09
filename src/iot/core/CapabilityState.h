#pragma once

#include "yh/iotcore_export.h"

#include <QVariantMap>

namespace Iot::State {
IOTCORE_EXPORT QVariantMap Boolean(const QString& instance, bool value);
IOTCORE_EXPORT QVariantMap OnOff(bool value);
IOTCORE_EXPORT QVariantMap WithRelative(QVariantMap state);
IOTCORE_EXPORT QVariantMap Toggle(const QString& instance, bool value);
IOTCORE_EXPORT QVariantMap Range(const QString& instance, double value);
IOTCORE_EXPORT QVariantMap RelativeRange(const QString& instance, double delta);
IOTCORE_EXPORT QVariantMap Mode(const QString& instance, const QString& value);
IOTCORE_EXPORT QVariantMap Color(const QString& instance, const QVariant& value);
IOTCORE_EXPORT QVariantMap Rgb(const QVariant& value);
IOTCORE_EXPORT QVariantMap Hsv(const QVariantMap& value);
IOTCORE_EXPORT QVariantMap Temperature(const QVariant& value);
IOTCORE_EXPORT QVariantMap Scene(const QString& value);
IOTCORE_EXPORT quint32 PackRgb(int red, int green, int blue);
IOTCORE_EXPORT QVariantMap HsvComponents(int hue, int saturation, int value);
}
