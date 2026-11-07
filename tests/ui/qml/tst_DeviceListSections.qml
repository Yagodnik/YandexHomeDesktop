import QtQuick
import QtTest
import YandexHomeDesktop.Components as Components

TestCase {
  id: testCase
  name: "DeviceListSections"
  width: 600
  height: 600
  visible: true
  when: windowShown

  ListModel { id: devices }

  Component {
    id: sectionComponent
    Components.FavoriteDevicesList {
      width: 350
      title: "Favorites"
      sourceModel: devices
      preferencesAvailable: true
      onCollapseToggled: function(value) { collapsed = value; }
    }
  }

  function init() {
    failOnWarning(/.?/);
    devices.clear();
  }

  function test_arrowFollowsTitleAndFitsLongNames() {
    const section = createTemporaryObject(sectionComponent, testCase);
    verify(section !== null);
    const header = findChild(section, "collapse-favorites");
    const title = findChild(header, "sectionTitle");
    const arrow = findChild(header, "sectionArrow");
    waitForPolish(section);
    compare(arrow.x - title.x - title.width, 8);
    compare(arrow.background, null);
    verify(arrow.x + arrow.width < section.width);

    section.title = "A very long room name that cannot fit in a narrow viewport";
    section.width = 150;
    waitForPolish(section);
    compare(arrow.x - title.x - title.width, 8);
    fuzzyCompare(arrow.x + arrow.width, section.width, 0.1);
    verify(title.truncated);
  }

  function test_emptySectionCanCollapseAndExpand() {
    const section = createTemporaryObject(sectionComponent, testCase, { collapsed: true });
    verify(section !== null);
    const body = findChild(section, "sectionBody-favorites");
    const header = findChild(section, "collapse-favorites");
    const arrow = findChild(header, "sectionArrow");
    const empty = findChild(section, "sectionEmpty-favorites");
    compare(body.height, 0);
    compare(arrow.rotation, 90);
    compare(section.height, header.height);
    mouseClick(header);
    tryVerify(function() { return body.height > 0 && body.height < body.expandedHeight; });
    tryCompare(arrow, "rotation", 180);
    tryCompare(body, "height", body.expandedHeight);
    verify(empty.visible);
    mouseClick(arrow);
    tryVerify(function() { return arrow.rotation > 90 && arrow.rotation < 180; });
    tryCompare(body, "height", 0);
    tryCompare(arrow, "rotation", 90);
    compare(section.height, header.height);
  }

  function test_animationCanReverseBeforeCompletion() {
    for (let i = 0; i < 3; ++i) {
      devices.append({ deviceId: "device-" + i, deviceType: "devices.types.light",
        name: "Lamp " + i, isFavorite: true });
    }
    const section = createTemporaryObject(sectionComponent, testCase);
    verify(section !== null);
    const body = findChild(section, "sectionBody-favorites");
    const header = findChild(section, "collapse-favorites");
    waitForPolish(section);
    tryCompare(body, "height", body.expandedHeight);
    mouseClick(header);
    tryVerify(function() { return body.height > 0 && body.height < body.expandedHeight; });
    mouseClick(header);
    compare(section.collapsed, false);
    tryCompare(body, "height", body.expandedHeight);
    tryCompare(findChild(header, "sectionArrow"), "rotation", 180);
  }
}
