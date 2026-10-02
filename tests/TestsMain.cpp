#include <QCoreApplication>
#include <QTest>
#include "ApiBoundaryTests.h"
#include "ApiTests.h"
#include "CapabilitiesTests.h"
#include "ModelBoundaryTests.h"
#include "ModelsTests.h"
#include "SerializationTests.h"

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  SerializationTests serialization_tests;
  ApiTests api_tests;
  ApiBoundaryTests api_boundary_tests;
  ModelBoundaryTests model_boundary_tests;
  CapabilitiesTests capabilities_tests;
  ModelsTests models_tests;

  int failed = 0;
  failed |= QTest::qExec(&serialization_tests, argc, argv);
  failed |= QTest::qExec(&api_tests, argc, argv);
  failed |= QTest::qExec(&api_boundary_tests, argc, argv);
  failed |= QTest::qExec(&model_boundary_tests, argc, argv);
  failed |= QTest::qExec(&capabilities_tests, argc, argv);
  failed |= QTest::qExec(&models_tests, argc, argv);

  return failed;
}
