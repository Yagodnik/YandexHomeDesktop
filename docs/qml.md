# QML UI guide

This describes the UI as it is wired in the current source tree. The QML lives in `src/qml/`; the C++ objects it uses are created in `src/main.cpp`.

## Entry point and file layout

`src/main.cpp` loads `YandexHomeDesktop.Main` with `QQmlApplicationEngine::loadFromModule()`. The root `src/qml/Main.qml` owns the window, tray icon, authorization signal handling, and page `StackView`. It registers route names and their `qrc:/pages/...` URLs before opening the `loading` route.

| Directory | Purpose | Packaging |
| --- | --- | --- |
| `src/qml/pages/` | Authorization, loading, error, main, device, and tab pages | Most are in the `YandexHomeDesktop.Pages` module; the routes used by `Main.qml` are also listed under `/pages` in `resources/resources.qrc`. `DevicePage.qml` is loaded through that resource list only. |
| `src/qml/components/` | Reusable app pieces such as the top bar, device rows, device header, and attribute list section | `YandexHomeDesktop.Components` module |
| `src/qml/ui/` | Shared text, buttons, switches, dialogs, card surfaces, refresh headers, and load states | `YandexHomeDesktop.Ui` module |
| `src/qml/controls/` | Capability and property controls for a selected device | Loaded by URL from `/controls` in `resources/resources.qrc`; `OnOff`, `Range`, `Mode`, and `Unsupported` are also in the `YandexHomeDesktop.IotControls` module. |

The module file lists are in `src/qml/CMakeLists.txt`. Images, fonts, theme JSON, and other data are declared in `resources/resources.qrc`. QML refers to those assets with `qrc:/...` URLs. The root `CMakeLists.txt` compiles `src/qml/shaders/highlight.frag` into the `/shaders` resource at build time.

## Navigation

`Main.qml` registers `loading`, `auth`, `main`, `error`, `device`, and `authCanceled` with the C++ `Router` (`src/utils/Router.*`). `Router::navigateTo()` invokes a QML helper that pushes the URL onto the `StackView`; `goBack()` pops it. The loading page attempts local authorization. Authorization signals route to the main, auth, error, or canceled pages.

`MainPage.qml` contains a `StackLayout` for the Devices, Scenarios, and Settings tabs, selected by `components/TopBar.qml`. These tabs are within the main page; they are not router destinations. A device row calls `deviceController.LoadDevice(deviceId)` and then navigates to the `device` route. The back button on `DevicePage.qml` forgets the selected device and pops the route.

The extracted visual components receive titles, models, and state through properties. `ui/RefreshHeader.qml` and `components/DeviceHeader.qml` emit click signals; their page handlers retain the refresh and back actions. `ui/LoadingPane.qml` and `ui/LoadErrorPane.qml` render the states chosen by each page. `components/DeviceAttributeSection.qml` renders either the capability or property model using its `delegateSource` role. `ui/CardSurface.qml` provides the shared rounded background used by cards and controls.

## C++ to QML data flow

`src/main.cpp` sets QML context properties for the shared objects. The ones most relevant to UI work are:

| Context property | Used for |
| --- | --- |
| `authorizationService`, `router`, `platformService` | Sign-in, page navigation, window and tray behavior |
| `devicesModel`, `roomsModel`, `householdsModel`, `scenariosModel`, `yandexAccount` | Lists, household selection, scenarios, account details |
| `deviceController`, `deviceDataModel`, `capabilitiesModel`, `propertiesModel` | Selected-device loading, polling, actions, and displayed attributes |
| `themes`, `settings` | Theme colors and persisted UI settings |
| `colorModel`, `colorModesModel`, `modesModel`, `iotTitles`, `eventTitles`, `unitsList`, `deviceIcons`, `propertiesIcons`, `errorCodes` | Control choices, labels, units, icons, and error messages |

`RegisterModels()` in `src/main.cpp` makes the filter models available through `YandexHomeDesktop.Models`. `RegisterCapabilities()` and `RegisterProperties()` expose the C++ attribute helpers used by controls through `YandexHomeDesktop.Capabilities` and `YandexHomeDesktop.Properties`.

`DevicesPage.qml` requests user info through `devicesModel.RequestData()`. The devices, rooms, and households models each consume the resulting API response. `RoomsFilterModel` filters by the current household, and each `RoomDevicesList` uses `DevicesFilterModel` for its room. `ScenariosPage.qml` requests its own scenario list and calls `scenariosModel.ExecuteScenario(index)` from a scenario row.

For a selected device, `DeviceController` receives device info and updates `capabilitiesModel` and `propertiesModel`. Both models expose a `delegateSource` role. `DevicePage.qml` uses that role as each `Loader.source`, so the model's URL map determines which QML control appears. Capability controls create an action with their C++ helper, then call `capabilitiesModel.UseCapability(model.index, action)`. Property controls display values from `Properties.Event` or `Properties.Float`. The window pauses and resumes device polling as it loses or gains activity.

## Where to make changes

- For a new routed page, add the QML file to `resources/resources.qrc` under `/pages` and register its route in `Main.qml`. Add it to `YandexHomeDesktop.Pages` in `src/qml/CMakeLists.txt` if another QML file will import it as a module type.
- For a new reusable component or visual primitive, add the QML file to the appropriate module in `src/qml/CMakeLists.txt` and import that module where needed.
- For a new device control, register its `qrc:/controls/...` resource, map the corresponding capability or property type to that URL in `CapabilitiesModel` or `PropertiesModel`, and use an appropriate C++ QML type for action or value handling. Add it to the `IotControls` module only if it also needs module import.
- For a new image, theme data file, or other bundled asset, add it to `resources/resources.qrc`. Reuse `themes` properties and `ui/` types for consistent colors and typography.

## Current source notes

- `DevicePage.qml` currently fixes its `StackLayout.currentIndex` at `2`; its loading and offline branches are present but bypassed.
- `Main.qml` currently displays an FPS text overlay in the window.
- QML files are not covered by the tests in `tests/`; those tests target C++ code.
- The clean build uses the credential-free auth template; see `docs/build.md` for local sign-in configuration.
