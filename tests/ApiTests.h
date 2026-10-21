#pragma once

#include <QtTest>
#include "api/model/Actions.h"
#include "api/model/Response.h"
#include "api/model/UserInfo.h"

class ApiTests final : public QObject {
  Q_OBJECT
private slots:
  static void TestCapabilities();
  static void CapabilitiesNullTest();
  static void CapabilitiesObjectTest();

  static void PropertiesTest();
  static void SharedResponseCodecs();
  static void SharedActionCodec();
};
