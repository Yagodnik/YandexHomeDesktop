#pragma once

#include <QByteArray>
#include <QCoreApplication>
#include <QString>

inline QString TranslateCatalog(const char *context, const QString &source) {
  const QByteArray utf8 = source.toUtf8();
  return QCoreApplication::translate(context, utf8.constData());
}
