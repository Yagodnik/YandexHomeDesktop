import QtQuick
import QtTest
import YandexHomeDesktop.Components as Components

TestCase {
  id: testCase
  name: "DeviceControlsLayout"
  width: 600
  height: 600
  when: windowShown

  ListModel { id: capabilities }
  ListModel { id: properties }

  Component {
    id: paneComponent

    Components.DeviceControlsPane {
      width: 350
      height: 120
      capabilitiesSourceModel: capabilities
      propertiesSourceModel: properties
      capabilitiesTitle: "Capabilities"
      propertiesTitle: "Properties"
    }
  }

  function init() {
    failOnWarning(/.?/);
    capabilities.clear();
    properties.clear();
  }

  function appendControl(sourceModel, controlId, controlHeight) {
    sourceModel.append({
      delegateSource: Qt.resolvedUrl("DeviceAttributeTestControl.qml").toString(),
      controlId: controlId,
      controlHeight: controlHeight
    });
  }

  function verifyBottomGap(pane, controlId) {
    const control = findChild(pane, controlId);
    verify(control !== null, "Expected the last device control to be loaded");
    tryVerify(function() {
      const bottom = control.mapToItem(pane.contentItem, 0, control.height).y;
      return Math.abs(pane.contentHeight - bottom - 12) < 0.1;
    }, 2000, "The scroll extent must include 12 pixels after the last control");

    if (pane.contentHeight > pane.height) {
      pane.contentY = pane.contentHeight - pane.height;
      const bottomInViewport = control.mapToItem(pane, 0, control.height).y;
      fuzzyCompare(bottomInViewport, pane.height - 12, 0.1);
    }
  }

  function test_bottomPadding_data() {
    return [
      { tag: "capabilities only", capabilityCount: 4, propertyCount: 0 },
      { tag: "properties only", capabilityCount: 0, propertyCount: 4 },
      { tag: "both sections", capabilityCount: 3, propertyCount: 2 },
      { tag: "short content", capabilityCount: 1, propertyCount: 0, viewportHeight: 500 },
      { tag: "many varying heights", capabilityCount: 30, propertyCount: 15 }
    ];
  }

  function test_bottomPadding(data) {
    let lastControlId = "";
    for (let i = 0; i < data.capabilityCount; ++i) {
      lastControlId = "capability" + i;
      appendControl(capabilities, lastControlId, 48 + (i % 3) * 50);
    }
    for (let i = 0; i < data.propertyCount; ++i) {
      lastControlId = "property" + i;
      appendControl(properties, lastControlId, 64);
    }

    const pane = createTemporaryObject(paneComponent, testCase, {
      height: data.viewportHeight ?? 120
    });
    verify(pane !== null);
    verifyBottomGap(pane, lastControlId);

    pane.width = 600;
    verifyBottomGap(pane, lastControlId);
  }

  function test_modelAndHeightChanges() {
    appendControl(capabilities, "capability", 90);
    appendControl(properties, "property", 64);
    const pane = createTemporaryObject(paneComponent, testCase);
    verify(pane !== null);
    verifyBottomGap(pane, "property");

    properties.clear();
    verifyBottomGap(pane, "capability");

    capabilities.setProperty(0, "controlHeight", 320);
    verifyBottomGap(pane, "capability");

    appendControl(properties, "newProperty", 64);
    verifyBottomGap(pane, "newProperty");
  }

  function test_emptyModels() {
    const pane = createTemporaryObject(paneComponent, testCase);
    verify(pane !== null);
    tryCompare(pane, "contentHeight", 16);
    compare(pane.contentWidth, pane.width);
  }
}
