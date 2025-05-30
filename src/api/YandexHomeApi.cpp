#include "YandexHomeApi.h"
#include <QDebug>

YandexHomeApi::YandexHomeApi(QObject *parent) : QObject(parent) {
  qDebug() << "YandexHomeApi::YandexHomeApi";
}