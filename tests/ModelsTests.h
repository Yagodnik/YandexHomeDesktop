#pragma once

#include <QtTest>
#include <QObject>

class ModelsTests final : public QObject {
  Q_OBJECT
private slots:
  static void TestColors();
};
