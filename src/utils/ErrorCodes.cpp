#include "ErrorCodes.h"

#include "JsonLoader.h"

ErrorCodes::ErrorCodes(QObject *parent) : QObject(parent) {
  const auto list = JsonLoader::Load<ErrorCodesFile>(kErrorCodesPath);

  if (list.has_value()) {
    const auto& actual_list = list.value();
    iot_errors_ = actual_list.error_codes;
  }
}

QVariant ErrorCodes::GetDeviceError(const QString &error_code) {
  for (const auto& error : iot_errors_) {
    if (error.error_code == error_code) {
      QVariantMap temp = {
        { "short_description", error.short_description },
        { "full_description", error.full_description }
      };

      return temp;
    }
  }

  qDebug() << "Unknown code: " << error_code;

  return {};
}

