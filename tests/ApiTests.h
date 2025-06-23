#pragma once

#include <QtTest>
#include "api/YandexHomeApi.h"

class ApiTests final : public QObject {
  Q_OBJECT
private slots:
  static void TestCapabilities();
  static void CapabilitiesNullTest();
  static void CapabilitiesObjectTest();
};