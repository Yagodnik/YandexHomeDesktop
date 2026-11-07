#include <QCoreApplication>
#include <QTest>
#include YH_TEST_HEADER

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  YH_TEST_CLASS tests;
  return QTest::qExec(&tests, argc, argv);
}
