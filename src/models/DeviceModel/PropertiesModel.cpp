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

void PropertiesModel::OnPropertiesUpdateReady(const QVariantList &properties2) {
  qDebug() << "PropertiesModel: Properties received:" << properties2.size() << "without fake data";

  QVariantList properties = properties2;

  if (!is_initialized_) {
    /* -- Adding fake data */

#ifdef ALLOW_FAKE_PROPETIES
    const int iterations = QRandomGenerator::global()->bounded(4) + 1;
#else
    const int iterations = 0;
#endif

    beginResetModel();
    properties_.resize(properties.size() + iterations);

    qDebug() << "Properties Model: added" << iterations << "fake properties";
  }

#ifdef ALLOW_FAKE_PROPETIES
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

      properties.push_back(QVariant::fromValue(Serialization::From<PropertyObject>(test_object)));
    }
  }
#endif

  qDebug() << "PropertiesModel: Updating" << properties.size() << "properties";

  for (auto [property, incoming_property_var] : std::ranges::views::zip(properties_, properties)) {
    const auto incoming_property = qvariant_cast<PropertyObject>(incoming_property_var);

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
