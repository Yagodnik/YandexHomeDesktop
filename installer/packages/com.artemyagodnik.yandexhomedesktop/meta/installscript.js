function Component()
{
  var installerMode = installer.value("mode");

  console.log("Running in " + installerMode);

  installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
}

Component.prototype.chooseTarget = function () {
  var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
  if (widget != null) {
    var newTarget = QFileDialog.getExistingDirectory("Choose your target directory.", widget
      .targetDirectory.text);
    if (newTarget != "")
    {
      newTarget += "/MyAppFolder";
      widget.targetDirectory.text = Dir.toNativeSparator(newTarget);
    }
  }
}

Component.prototype.createOperations = function()
{
  try {
    component.createOperations();

    if (installer.value("os") == "win") {
      try {
        component.addOperation("CreateShortcut", "@TargetDir@\\YandexLampTray.exe", "@StartMenuDir@/YandexHomeWidgets.lnk");
      } catch (e) {
        print(e)
      }
    }
  } catch (e) {
    print(e);
  }
}