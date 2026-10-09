# QML UI guide

This describes the UI as it is wired in the current source tree. The QML lives in `src/qml/`; `src/app/GuiApp.cpp` creates its models and exposes the shared services from `AppContext`.

## Entry point and file layout

`GuiApp::Start()` loads `YandexHomeDesktop.Main` with `QQmlApplicationEngine::loadFromModule()`. The root `src/qml/Main.qml` owns the window, tray icon, authorization signal handling, and page `StackView`. It registers route names and their `qrc:/pages/...` URLs before opening the `loading` route.

| Directory | Purpose | Packaging |
| --- | --- | --- |
| `src/qml/pages/` | Authorization, loading, error, main, device, and tab pages | All are in the `YandexHomeDesktop.Pages` module; the routes used by `Main.qml` are also listed under `/pages` in `resources/resources.qrc`. |
| `src/qml/components/` | Reusable app pieces such as the top bar, device rows, device header, and attribute list section | `YandexHomeDesktop.Components` module |
| `src/qml/ui/` | Shared text, buttons, switches, dialogs, card surfaces, refresh headers, and load states | `YandexHomeDesktop.Ui` module |
| `src/qml/controls/` | Capability and property controls for a selected device | Loaded by URL from `/controls` in `resources/resources.qrc`. |

The module file lists are in `src/qml/CMakeLists.txt`. Images, fonts, theme JSON, and other data are declared in `resources/resources.qrc`. QML refers to those assets with `qrc:/...` URLs.

## Navigation

`Main.qml` registers `loading`, `auth`, `main`, `error`, `device`, and `authCanceled` with the C++ `Router` (`src/utils/Router.*`). `Router::navigateTo()` invokes a QML helper that pushes the URL onto the `StackView`; `goBack()` pops it. The loading page attempts local authorization. Authorization signals route to the main, auth, error, or canceled pages.

`MainPage.qml` uses `ui/PageStates.qml` for the Devices, Scenarios, and Settings tabs, selected by `components/TopBar.qml`. These tabs are within the main page; they are not router destinations. A device row calls `deviceViewModel.LoadDevice(deviceId)` and then navigates to the `device` route. The back button on `DevicePage.qml` forgets the selected device and pops the route. Device page state binds to the view model, so it also works when loading completes before navigation.

## Page composition

Pages compose `Ui` and `Components` types rather than defining rectangles, mouse areas, lists, or inline control delegates. Keep translated labels and the model/controller event handlers in the pages. Components receive display data through properties and report user intent through signals; they do not reach into a page's IDs.

| Component | Responsibility |
| --- | --- |
| `ui/PageSurface.qml`, `ui/PageStates.qml`, `ui/InsetPane.qml` | Page background, state/tab layout, and clipped content with consistent insets |
| `ui/ScrollColumn.qml`, `ui/ListScrollBar.qml` | Measured vertical content and the shared list scrollbar |
| `ui/Shadow.qml` | Shared shadow parameters and deferred shader effects when a graphics backend is available |
| `ui/LoadingPane.qml`, `ui/RetryPane.qml`, `ui/MessageActionsPane.qml` | Loading, retry, and authorization/error messages with action signals |
| `components/SignInCard.qml`, `ui/LinkFooter.qml` | Sign-in presentation and footer link |
| `components/HouseholdPicker.qml`, `components/HouseholdDelegate.qml` | Selection sheet, backdrop, and household rows |
| `components/BasicSettingsCard.qml`, `ui/Setting*Row.qml` | Settings presentation; the page applies tray and theme changes |
| `components/RoomsPane.qml`, `components/ScenariosPane.qml` | List viewports; the scenarios pane forwards execution requests to its page |

`TopBar.householdSelectRequested` is handled by `MainPage`, which opens the picker. The picker emits `householdSelected`; the page calls `homeViewModel.SelectHousehold(id)` and closes the sheet. The scenarios viewport uses one scrolling list with an attached scrollbar. Settings uses `ScrollColumn` so the scroll extent follows the actual content height.

The extracted visual components receive titles, models, and state through properties. `ui/RefreshHeader.qml` and `components/DeviceHeader.qml` emit click signals; their page handlers retain the refresh and back actions. `ui/LoadingPane.qml` and `ui/LoadErrorPane.qml` render the states chosen by each page. `components/DeviceControlsPane.qml` owns the selected device's scrolling and padding, including the bottom gap when either attribute section is empty. `components/DeviceAttributeSection.qml` renders either model using its `delegateSource` role; a column of repeated loaders measures every control's height without a nested scrolling list. `components/PropertyValueCard.qml` shares the icon, value, and title layout between float and event controls, which retain their own property helpers and model bindings. `ui/CardSurface.qml` provides the shared rounded background used by cards and controls.

## C++ to QML data flow

`src/app/GuiApp.cpp` sets QML context properties for the shared objects. The ones most relevant to UI work are:

| Context property | Used for |
| --- | --- |
| `authorizationService`, `router`, `platformService` | `AuthorizationModel` sign-in commands/signals, page navigation, window and tray behavior |
| `homeViewModel`, `scenariosViewModel`, `yandexAccount` | Home lists and household selection, scenario operations, account details |
| `deviceViewModel` | Selected-device loading, page state, actions, and owned display models |
| `deviceController`, `deviceDataModel`, `capabilitiesModel`, `propertiesModel` | Compatibility aliases for the device view model and its models |
| `themes`, `settings` | Theme colors and persisted UI settings |
| `colorModel`, `colorModesModel`, `modesModel`, `iotTitles`, `eventTitles`, `unitsList`, `deviceIcons`, `propertiesIcons`, `errorCodes` | Control choices, labels, units, icons, and error messages |

`GuiApp::RegisterModels()` makes the filter models available through `YandexHomeDesktop.Models`. `RegisterCapabilities()` and `RegisterProperties()` expose the C++ attribute helpers used by controls through `YandexHomeDesktop.Capabilities` and `YandexHomeDesktop.Properties`.

`DevicesPage.qml` receives a required `HomeViewModel` and calls `EnsureLoaded()` or `Refresh()`. The view model exposes page state, household selection, and the devices/rooms/households lists. `HomeService` in `src/services` owns the cached snapshot, selection validation, request correlation, and session reset. The three list models expose rows and notifications. The view model owns a `RoomsFilterModel` for the selected household, while each `RoomDevicesList` receives the device list explicitly and uses `DevicesFilterModel` for its room. Loading and error visuals bind to the view model state.

Rooms and Favorites share `DeviceListSection` and `CollapsibleSectionHeader`. The bundled arrow sits eight pixels after the title, has no background, and rotates over 220 ms. Section bodies animate their height and opacity over the same duration, clip disappearing rows, and disable interaction while collapsing. `RoomsModel.isCollapsed` controls rooms; `HomeViewModel.favoritesCollapsed` controls the fixed Favorites section, which stays visible with an empty message when it has no devices. `RoomsFilterModel` sorts names lexicographically with a case-sensitive, locale-independent comparison and uses room ID to break ties, so API response order does not affect the list.

Each `DeviceDelegate` includes `ui/FavoriteButton`; `DevicesModel.isFavorite` keeps the star synchronized in room and favorite rows. `RoomsPane` places `FavoriteDevicesList` above the rooms in one `ScrollColumn`, with repeated columns measuring every room and device row. The viewport keeps its scroll position as sections expand; a room taller than the viewport does not trigger automatic list positioning or estimated-height jumps. Favorites belong to the selected household and follow addition order; removing and adding a device again places it last. They also remain in their room lists.

These components emit collapse, favorite, and device-selection signals. `DevicesPage` handles them through `HomeViewModel` and the existing device navigation. The view model saves favorite IDs, collapsed room IDs, and the Favorites collapse flag through `Settings`, under a hashed stable account ID supplied by `AccountModel`. Controls become available once that ID is loaded, and refreshing retries account loading after a failure. Logout clears the current account state without deleting saved preferences. Names, email addresses, and OAuth tokens are not used as preference keys. Devices missing from a refreshed snapshot are hidden from favorites while their saved positions are retained for a later snapshot. Fixture mode keeps these preferences in memory.

`ScenariosPage.qml` receives a required `ScenariosViewModel`, binds its state and list, and calls `ExecuteScenario(id)`. `ScenarioService` owns execution guards, pending requests, and session reset; `ScenariosModel` provides the row roles. `GuiApp` constructs the view models, and `AppContext` supplies services using `IHomeApi`. Logout resets both services. Debug fixture mode supplies the same interfaces with local JSON data; see `docs/build.md`.

For a selected device, `DeviceViewModel` projects updates from its `DeviceSession` into `capabilitiesModel` and `propertiesModel`. Both models expose a `delegateSource` role. `DeviceAttributeSection.qml` uses that role as each `Loader.source`, so the model's URL map determines which QML control appears. Capability controls create an action with their C++ helper, then call `capabilitiesModel.UseCapability(model.index, action)`. The model updates the displayed state and emits a request that the view model routes through the session and `DeviceService`. Property controls display values from `Properties.Event` or `Properties.Float`. The window pauses and resumes device polling as it loses or gains activity. See [device-control-architecture.md](device-control-architecture.md) for service boundaries and the preserved conflict behavior.

## Where to make changes

- For a new routed page, add the QML file to `resources/resources.qrc` under `/pages` and register its route in `Main.qml`. Add it to `YandexHomeDesktop.Pages` in `src/qml/CMakeLists.txt` if another QML file will import it as a module type.
- For a new reusable component or visual primitive, add the QML file to the appropriate module in `src/qml/CMakeLists.txt` and import that module where needed.
- For a new device control, register its `qrc:/controls/...` resource, map the corresponding capability or property type to that URL in `CapabilitiesModel` or `PropertiesModel`, and use an appropriate C++ QML type for action or value handling.
- For a new image, theme data file, or other bundled asset, add it to `resources/resources.qrc`. Reuse `themes` properties and `ui/` types for consistent colors and typography.

## Current source notes

- `DevicePage.qml` starts in the loading state; its model initialization signals select the loaded or offline state.
- The FPS overlay in `Main.qml` is commented out.
- `tests/qml/` covers device-control spacing, settings scrolling and actions, scenario scrolling and execution signals, household selection, sign-in, and device retry/initialization. Desktop builds also create every page with local test models and fail on QML warnings. These checks run through CTest; see `docs/build.md`.
- The clean build uses the credential-free auth template; see `docs/build.md` for local sign-in configuration.
