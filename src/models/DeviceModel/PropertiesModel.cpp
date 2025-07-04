#include "PropertiesModel.h"

#include <QRandomGenerator>

#include "CapabilitiesModel.h"

PropertiesModel::PropertiesModel(YandexHomeApi* api, QObject* parent)
  : QAbstractListModel(parent), api_(api)
{
  connect(api_,
    &YandexHomeApi::deviceInfoReceived,
    this,
    &PropertiesModel::OnDeviceInfoReceived);

  connect(api_,
    &YandexHomeApi::deviceInfoReceivingFailed,
    this,
    &PropertiesModel::OnDeviceInfoReceivingFailed);
}

void PropertiesModel::ResetModel() {
  beginResetModel();

  properties_.clear();
  is_initialized_ = false;

  endResetModel();
}

int PropertiesModel::rowCount(const QModelIndex &parent) const {
  return properties_.size();
}

QVariant PropertiesModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= properties_.size()) {
    return {};
  }

  const auto& property = properties_.at(index.row());

  switch (role) {
    case IdRole:
      return {};
    case NameRole:
      return PropertyType::operator[](property.type);
    case StateRole:
      return property.state;
    case ParametersRole:
      return property.parameters;
    case DelegateSourceRole:
      switch (property.type) {
      case PropertyType::Float:
          return "qrc:/controls/PropertyFloat.qml";
      case PropertyType::Event:
          return "qrc:/controls/PropertyEvent.qml";
      }
    default:
      return {};
  }
}

QHash<int, QByteArray> PropertiesModel::roleNames() const {
  return {
    { IdRole, "deviceId" },
    { NameRole, "name"},
    { StateRole, "propertyState" },
    { ParametersRole, "propertyParameters" },
    { DelegateSourceRole, "delegateSource" }
  };
}

void PropertiesModel::OnDeviceInfoReceived(const DeviceInfo &info2) {
  qDebug() << "Updated by timer from another model!";

  qDebug() << "Properties received:" << info2.properties.size() << "without fake data";

  /* -- Fake data for testing -- */
  DeviceInfo info = info2;

  qDebug() << "Properties received (copy):" << info.properties.size() << "without fake data";

  {
    QString pseudo_property_data_str = R"(
    {
        "type": "devices.properties.float",
        "retrievable": true,
        "parameters": {
            "instance": "humidity",
            "unit": "unit.percent"
        },
        "state": {
            "instance": "humidity",
            "value": %1
        }
    })";

    pseudo_property_data_str = pseudo_property_data_str.arg(QRandomGenerator::global()->bounded(101));
    const QJsonObject test_object = QJsonDocument::fromJson(pseudo_property_data_str.toUtf8()).object();

    info.properties.push_back(Serialization::From<PropertyObject>(test_object));
  }

  {
    QString pseudo_property_data_str = R"({
      "type": "devices.properties.event",
      "retrievable": true,
      "reportable": true,
      "parameters": {
        "instance": "open",
        "events": [
          { "value": "opened" },
          { "value": "closed" }
        ]
      },
      "state": {
          "instance": "open",
          "value": "%1"
      }
    })";

    pseudo_property_data_str = pseudo_property_data_str.arg(
      QRandomGenerator::global()->bounded(101) > 70 ? "opened" : "closed"
    );
    const QJsonObject test_object = QJsonDocument::fromJson(pseudo_property_data_str.toUtf8()).object();

    info.properties.push_back(Serialization::From<PropertyObject>(test_object));
  }

  if (!is_initialized_) {
    beginResetModel();
    properties_.resize(info.properties.size());
  }

  qDebug() << "Updating" << info.properties.size() << "properties";

  for (auto [property, incoming_property] : std::ranges::views::zip(properties_, info.properties)) {
    qDebug() << "Property update:"  << incoming_property.state;
    qDebug() << "\t"  << incoming_property.parameters;

    property = incoming_property;
  }

  if (!is_initialized_) {
    emit initialized();
    endResetModel();
  }

  is_initialized_ = true;

  const auto top_left = createIndex(0, 0);
  const auto bottom_right = createIndex(properties_.size() - 1, 0);

  emit dataChanged(top_left, bottom_right);
  emit dataLoaded();
}

void PropertiesModel::OnDeviceInfoReceivingFailed(const QString &message) {
  qDebug() << "Devices Model: Error receiving devices:" << message;

  emit dataLoadingFailed();
}
