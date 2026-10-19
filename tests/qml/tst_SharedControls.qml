import QtQuick
import QtQuick.Window
import QtTest
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

TestCase {
  id: testCase
  name: "SharedControls"
  width: 600
  height: 600
  visible: true
  when: windowShown

  Component { id: button; UI.MyButton { text: "Action" } }
  Component { id: imageButton; UI.ImageButton { width: 32; height: 32; source: "qrc:/images/arrow.svg" } }
  Component { id: toggle; UI.MySwitch {} }
  Component { id: tab; UI.MyTab { text: "Tab" } }
  Component { id: link; UI.LinkLabel { text: "Link" } }
  Component { id: favorite; UI.FavoriteButton {} }
  Component { id: keybind; Components.CapabilityKeybind { capabilityTitle: "Action" } }
  Component { id: section; Components.CollapsibleSectionHeader { width: 300; title: "Room"; collapsed: false } }
  Component { id: tile; Components.CapabilityOptionTile { width: 100; title: "Color"; showSwatch: true; swatchColor: "red" } }
  Component { id: topBar; Components.TopBar { householdName: "Home" } }
  Component { id: household; Components.HouseholdDelegate { width: 250; title: "Home" } }
  Component { id: combo; UI.MyComboBox { model: ["Short", "A very long option that must stay on one line", "Тёмная"] } }

  SignalSpy { id: selectedSpy; signalName: "activated" }

  function init() {
    failOnWarning(/.?/);
    testCase.Window.window.width = 600;
    testCase.Window.window.height = 600;
    themes.SetTheme(0);
    selectedSpy.clear();
  }

  function verifyHandCursor(item) {
    mouseMove(testCase, 580, 580);
    mouseMove(item, item.width / 2, item.height / 2);
    tryVerify(function() {
      return uiTestHelper.cursorShape(testCase.Window.window) === Qt.PointingHandCursor;
    }, 1000, "Hovering an action must show the pointing hand");
  }

  function test_actionCursors_data() {
    return [
      { tag: "button", control: button },
      { tag: "image button", control: imageButton },
      { tag: "switch", control: toggle },
      { tag: "tab", control: tab },
      { tag: "link", control: link },
      { tag: "favorite", control: favorite },
      { tag: "keybind", control: keybind },
      { tag: "section", control: section },
      { tag: "section arrow", control: section, child: "sectionArrow" },
      { tag: "color tile", control: tile },
      { tag: "home selector", control: topBar, child: "householdSelector" },
      { tag: "devices menu", control: topBar, child: "devicesTab" },
      { tag: "scenarios menu", control: topBar, child: "scenariosTab" },
      { tag: "settings menu", control: topBar, child: "settingsTab" },
      { tag: "home menu item", control: household },
      { tag: "combobox", control: combo }
    ];
  }

  function test_actionCursors(data) {
    const control = createTemporaryObject(data.control, testCase, { x: 20, y: 20 });
    verify(control !== null);
    waitForRendering(control);
    verifyHandCursor(data.child ? findChild(control, data.child) : control);
  }

  function test_comboBoxTextAndInteraction_data() {
    return [{ tag: "light", theme: 0, pickerWidth: 80 }, { tag: "dark", theme: 1, pickerWidth: 120 }];
  }

  function test_comboBoxTextAndInteraction(data) {
    themes.SetTheme(data.theme);
    const picker = createTemporaryObject(combo, testCase, {
      x: 20, y: 20, width: data.pickerWidth, currentIndex: 1
    });
    selectedSpy.target = picker;
    const label = picker.contentItem;
    tryVerify(function() { return label.truncated; });
    compare(label.lineCount, 1);
    verify(label.x >= 8 && label.x + label.width <= picker.width - 8);
    verify(label.y >= 0 && label.y + label.height <= picker.height);
    verifyHandCursor(picker);
    mouseClick(picker);
    tryCompare(picker.popup, "opened", true);
    const longOption = findChild(picker.popup.contentItem, "comboBoxOption-1");
    verify(longOption !== null);
    const optionLabel = findChild(longOption, "comboBoxOptionLabel");
    compare(optionLabel.font.pixelSize, label.font.pixelSize);
    compare(optionLabel.lineCount, 1);
    verify(optionLabel.truncated);
    verify(optionLabel.x + optionLabel.width <= longOption.width - 8);
    verifyHandCursor(longOption);
    if (settingsPreviewPath.length > 0)
      grabImage(testCase.Window.window.contentItem).save(settingsPreviewPath + "-combo-" + data.tag + ".png");

    const firstOption = findChild(picker.popup.contentItem, "comboBoxOption-0");
    mouseClick(firstOption);
    tryCompare(picker.popup, "visible", false);
    compare(picker.currentIndex, 0);
    compare(selectedSpy.count, 1);
    compare(selectedSpy.signalArguments[0][0], 0);
    picker.forceActiveFocus();
    keyClick(Qt.Key_Down);
    compare(picker.currentIndex, 1);
    compare(selectedSpy.count, 2);
    compare(selectedSpy.signalArguments[1][0], 1);
  }
}
