#pragma once

#include <QObject>

#include "../serialization/Serialization.h"

class ErrorCodes : public QObject {
  Q_OBJECT
public:
  explicit ErrorCodes(QObject *parent = nullptr);

  Q_INVOKABLE [[nodiscard]] QVariant GetDeviceError(const QString& error_code);

private:
  JSON_STRUCT(ErrorCode,
    (QString, error_code),
    (QString, short_description),
    (QString, full_description)
  );

  JSON_STRUCT(ErrorCodesFile,
    (QList<ErrorCode>, error_codes)
  );

  const QString kErrorCodesPath = ":/data/errorCodes.json";

  QList<ErrorCode> iot_errors_;
};
