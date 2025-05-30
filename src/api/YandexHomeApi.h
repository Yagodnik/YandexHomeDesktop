#ifndef YANDEXHOMEAPI_H
#define YANDEXHOMEAPI_H

#include <QObject>

class YandexHomeApi final : QObject {
public:
  explicit YandexHomeApi(QObject *parent = nullptr);
};

#endif //YANDEXHOMEAPI_H
