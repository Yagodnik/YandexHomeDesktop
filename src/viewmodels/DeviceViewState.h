#pragma once

#include <QObject>

class DeviceViewState : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool showErrorMessage READ GetShowErrorMessage WRITE SetShowErrorMessage NOTIFY showErrorMessageChanged)
  Q_PROPERTY(QString errorMessage READ GetErrorMessage WRITE SetErrorMessage NOTIFY errorMessageChanged)
  Q_PROPERTY(int currentPage READ GetCurrentPage WRITE SetCurrentPage NOTIFY currentPageChanged)
  Q_PROPERTY(bool hasCapabilities READ HasCapabilities WRITE SetHasCapabilities NOTIFY hasCapabilitiesChanged)
  Q_PROPERTY(bool hasProperties READ HasProperties WRITE SetHasProperties NOTIFY hasPropertiesChanged)
  Q_PROPERTY(bool deviceOnline READ IsDeviceOnline WRITE SetDeviceOnline NOTIFY deviceOnlineChanged)
public:
  explicit DeviceViewState(QObject* parent = nullptr);

  [[nodiscard]] bool GetShowErrorMessage() const;
  void SetShowErrorMessage(bool show_error_message);

  [[nodiscard]] const QString& GetErrorMessage() const;
  void SetErrorMessage(const QString& error_message);

  [[nodiscard]] int GetCurrentPage() const;
  void SetCurrentPage(int current_page);

  [[nodiscard]] bool HasCapabilities() const;
  void SetHasCapabilities(bool has_capabilities);

  [[nodiscard]] bool HasProperties() const;
  void SetHasProperties(bool has_properties);

  [[nodiscard]] bool IsDeviceOnline() const;
  void SetDeviceOnline(bool device_online);

signals:
  void showErrorMessageChanged();
  void errorMessageChanged();
  void currentPageChanged();
  void hasCapabilitiesChanged();
  void hasPropertiesChanged();
  void deviceOnlineChanged();

private:
  bool show_error_message_ {false};
  QString error_message_;
  int current_page_{};

  bool has_capabilities_ {false};
  bool has_properties_ {false};
  bool device_online_ {false};
};
