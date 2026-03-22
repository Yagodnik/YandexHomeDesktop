#pragma once

#include <QUrl>

class INetworkClient {
public:
  virtual ~INetworkClient() = default;

  virtual void Get(const QUrl& endpoint) = 0;
  virtual void Post(const QUrl& endpoint) = 0;
};
