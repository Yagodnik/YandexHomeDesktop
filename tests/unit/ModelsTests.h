#pragma once

#include <QtTest>
#include <QObject>

class ModelsTests final : public QObject {
  Q_OBJECT
private slots:
  static void TestColors();
  static void WindowPlacementEdges();
  static void NotchActivationArea();
  static void NotchActivationWindowStates();
};
