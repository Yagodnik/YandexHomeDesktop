import QtQuick
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.Properties as Properties

Components.PropertyValueCard {
  iconSource: propertiesIcons.GetIcon(eventProperty.instance)
  valueText: eventProperty.formattedValue
  titleText: eventProperty.title + (model.updateTime !== null ? (" • " + model.updateTime) : "")

  Properties.Event {
    id: eventProperty

    state: model.propertyState
    parameters: model.propertyParameters
    titlesList: iotTitles
    valuesTitles: eventTitles
  }
}
