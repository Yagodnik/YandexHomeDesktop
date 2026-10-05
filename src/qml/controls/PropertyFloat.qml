import QtQuick
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.Properties as Properties

Components.PropertyValueCard {
  iconSource: propertiesIcons.GetIcon(floatProperty.instance)
  valueText: floatProperty.formattedValue
  titleText: floatProperty.title + (model.updateTime !== null ? (" • " + model.updateTime) : "")

  Properties.Float {
    id: floatProperty

    state: model.propertyState
    parameters: model.propertyParameters
    titlesList: iotTitles
    units: unitsList
  }
}
