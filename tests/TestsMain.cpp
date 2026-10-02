#include <QCoreApplication>
#include <QTest>
#include "ApiTests.h"
#include "CapabilitiesTests.h"
#include "ModelsTests.h"
#include "SerializationTests.h"

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  SerializationTests serialization_tests;
  ApiTests api_tests;
  CapabilitiesTests capabilities_tests;
  ModelsTests models_tests;

  int failed = 0;
  failed |= QTest::qExec(&serialization_tests, argc, argv);
  failed |= QTest::qExec(&api_tests, argc, argv);
  failed |= QTest::qExec(&capabilities_tests, argc, argv);
  failed |= QTest::qExec(&models_tests, argc, argv);

  return failed;
}
