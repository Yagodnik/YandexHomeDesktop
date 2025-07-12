#include <QTest>
#include "ApiTests.h"
#include "CapabilitiesTests.h"
#include "ModelsTests.h"
#include "SerializationTests.h"

int main(int argc, char *argv[]) {
  int failed = 0;
  failed |= QTest::qExec(new SerializationTests(), argc, argv);
  failed |= QTest::qExec(new ApiTests(), argc, argv);
  failed |= QTest::qExec(new CapabilitiesTests(), argc, argv);
  failed |= QTest::qExec(new ModelsTests(), argc, argv);

  return failed;
}
