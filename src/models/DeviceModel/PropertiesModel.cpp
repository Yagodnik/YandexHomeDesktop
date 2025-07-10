#include "PropertiesModel.h"

#include <QRandomGenerator>

#include "CapabilitiesModel.h"

PropertiesModel::PropertiesModel(DeviceController* controller, QObject* parent)
  : QAbstractListModel(parent), controller_(controller)
{
  connect(controller,
    &DeviceController::loadRequestMade,
    this,
    &PropertiesModel::ResetModel);

  connect(controller,
    &DeviceController::propertiesUpdateReady,
    this,
    &PropertiesModel::OnPropertiesUpdateReady);
}

void PropertiesModel::ResetModel() {
  qDebug() << "Properties Model: Reset due to controller request";

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
      default:
          return "qrc:/controls/Unsupported.qml";
      }
    case UpdateTimeRole: {
      // TODO: Add support to optional types to get rid from this mess...
      // Just checking if it is somehow initialized
      if (property.last_updated < 1) {
        return QVariant::fromValue(nullptr);
      }

      const QDateTime date_time = QDateTime::fromSecsSinceEpoch(property.last_updated).toLocalTime();

      QString output;
      const QDate today = QDate::currentDate();

      if (date_time.date() == today) {
        output = date_time.toString("HH:mm");
      } else {
        output = date_time.toString("yyyy-MM-dd HH:mm");
      }

      return output;
    } default:
      return {};
  }
}

QHash<int, QByteArray> PropertiesModel::roleNames() const {
  return {
    { IdRole, "deviceId" },
    { NameRole, "name"},
    { StateRole, "propertyState" },
    { ParametersRole, "propertyParameters" },
    { DelegateSourceRole, "delegateSource" },
    { UpdateTimeRole, "updateTime" }
  };
}

void PropertiesModel::OnPropertiesUpdateReady(const DeviceController::PropertiesList &properties2) {
  qDebug() << "PropertiesModel: Properties received:" << properties2.size() << "without fake data";

  DeviceController::PropertiesList properties = properties2;

  if (!is_initialized_) {
    /* -- Adding fake data */

#ifdef ALLOW_FAKE_PROPERTIES
    const int iterations = QRandomGenerator::global()->bounded(4) + 1;
#else
    const int iterations = 0;
#endif

    beginResetModel();
    properties_.resize(properties.size() + iterations);

    qDebug() << "Properties Model: added" << iterations << "fake properties";
  }

#ifdef ALLOW_FAKE_PROPERTIES
  for (int i = 0; i < properties_.size(); ++i) {
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

      properties.push_back(Serialization::From<PropertyObject>(test_object));
    }
  }
#endif

  qDebug() << "PropertiesModel: Updating" << properties.size() << "properties";

  for (auto [property, incoming_property_opt] : std::ranges::views::zip(properties_, properties)) {
    if (!incoming_property_opt.has_value()) {
      qDebug() << "PropertiesModel: Updating property failed as it is nullopt!";
      continue;
    }

    const auto incoming_property = incoming_property_opt.value();

    qDebug() << "PropertiesModel: Property update:"  << incoming_property.state;
    qDebug() << "PropertiesModel: Property Params:"  << incoming_property.parameters;

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
