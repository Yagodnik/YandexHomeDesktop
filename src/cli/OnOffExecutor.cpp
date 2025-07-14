#include "OnOffExecutor.h"

#include <QDebug>

void OnOffExecutor::Execute(const QString& value) {
  qDebug() << "OnOffExecutor::Execute(" << value << ")";
}

void OnOffExecutor::PrintInfo() {
  qDebug() << "On Off Capability Info";
  qDebug() << "Sets on off value";
}
