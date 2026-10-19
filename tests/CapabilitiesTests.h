#pragma once

#include <QtTest>
#include <QObject>

class CapabilitiesTests final : public QObject {
  Q_OBJECT
private slots:
  static void TestOnOff();
  static void TestColorSetting();
  static void CurrentColorMatchesOnlyTheActiveColorSpace();
  static void TestRange();
  static void TestToggle();
  static void SharedBuildersPreserveDesktopPayloads();
  static void SharedParametersPreserveDesktopDefaults();
};
