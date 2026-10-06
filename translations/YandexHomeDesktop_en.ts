<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en" sourcelanguage="ru">
<context>
    <name>AuthCanceledPage</name>
    <message>
        <location filename="../src/qml/pages/AuthCanceledPage.qml" line="7"/>
        <source>Не удалось получить доступ к хранилищу!</source>
        <translation>Could not access secure storage!</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/AuthCanceledPage.qml" line="9"/>
        <source>Это необходимо для работы приложения</source>
        <translation>The app needs it to work</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/AuthCanceledPage.qml" line="10"/>
        <source>Попробовать ещё раз</source>
        <translation>Try again</translation>
    </message>
</context>
<context>
    <name>AuthPage</name>
    <message>
        <location filename="../src/qml/pages/AuthPage.qml" line="16"/>
        <source>Yandex Home Desktop</source>
        <translation>Yandex Home Desktop</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/AuthPage.qml" line="17"/>
        <source>Необходимо войти в аккаунт, чтобы
приложение могло получить доступ
к вашим устройствам</source>
        <translation>Sign in so the app can access
your devices</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/AuthPage.qml" line="18"/>
        <source>Авторизоваться</source>
        <translation>Sign in</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/AuthPage.qml" line="26"/>
        <source>GitHub</source>
        <translation>GitHub</translation>
    </message>
</context>
<context>
    <name>CapabilitiesModel</name>
    <message>
        <location filename="../src/models/DeviceModel/CapabilitiesModel.cpp" line="33"/>
        <source>Неизвестный тип умения</source>
        <translation>Unknown capability type</translation>
    </message>
</context>
<context>
    <name>CapabilityCommands</name>
    <message>
        <source>Значение должно быть конечным числом.</source>
        <translation type="vanished">The value must be a finite number.</translation>
    </message>
    <message>
        <source>Значение выходит за допустимые границы устройства.</source>
        <translation type="vanished">The value is outside the device limits.</translation>
    </message>
    <message>
        <source>Укажите --instance; для on_off используется on.</source>
        <translation type="vanished">Specify --instance; on_off uses on.</translation>
    </message>
    <message>
        <source>Логическое значение должно быть on/off или true/false.</source>
        <translation type="vanished">A boolean value must be on/off or true/false.</translation>
    </message>
    <message>
        <source>Для этого умения требуется --instance.</source>
        <translation type="vanished">This capability requires --instance.</translation>
    </message>
    <message>
        <source>Это range поддерживает только --relative.</source>
        <translation type="vanished">This range only supports --relative.</translation>
    </message>
    <message>
        <source>Для mode требуются непустые --instance и --value.</source>
        <translation type="vanished">Mode requires nonempty --instance and --value.</translation>
    </message>
    <message>
        <source>Устройство не поддерживает такой режим или сцену.</source>
        <translation type="vanished">The device does not support this mode or scene.</translation>
    </message>
    <message>
        <source>Режим или сцена не должны быть пустыми.</source>
        <translation type="vanished">The mode or scene must not be empty.</translation>
    </message>
    <message>
        <source>HSV должен быть JSON-объектом с h, s и v.</source>
        <translation type="vanished">HSV must be a JSON object with h, s, and v.</translation>
    </message>
    <message>
        <source>HSV: h должен быть 0–360; s и v — 0–100.</source>
        <translation type="vanished">HSV: h must be 0–360; s and v must be 0–100.</translation>
    </message>
    <message>
        <source>Цвет: rgb — целое 0–16777215; temperature_k — положительное целое; также доступны hsv и scene.</source>
        <translation type="vanished">Color: rgb must be an integer from 0–16777215; temperature_k must be a positive integer; hsv and scene are also available.</translation>
    </message>
    <message>
        <location filename="../src/cli/CapabilityCommands.cpp" line="14"/>
        <source>Укажите поддерживаемый --capability и --value.</source>
        <translation>Specify a supported --capability and --value.</translation>
    </message>
    <message>
        <source>--relative поддерживается только для range.</source>
        <translation type="vanished">--relative is only supported for range.</translation>
    </message>
</context>
<context>
    <name>CapabilityMessages</name>
    <message>
        <location filename="../src/iot/core/Validation.cpp" line="14"/>
        <source>Для этого умения требуется экземпляр.</source>
        <translation>This capability requires an instance.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/Validation.cpp" line="20"/>
        <source>Значение выходит за допустимые границы устройства.</source>
        <translation>The value is outside the device limits.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/Validation.cpp" line="26"/>
        <source>Относительное изменение поддерживается только для range.</source>
        <translation>Relative changes are only supported for range.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/BooleanRules.cpp" line="13"/>
        <source>Укажите экземпляр умения; для on_off используется on.</source>
        <translation>Specify a capability instance; on_off uses on.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/BooleanRules.cpp" line="16"/>
        <source>Логическое значение должно быть true или false.</source>
        <translation>A boolean value must be true or false.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/RangeRules.cpp" line="13"/>
        <location filename="../src/iot/core/ColorRules.cpp" line="13"/>
        <source>Значение должно быть конечным числом.</source>
        <translation>The value must be a finite number.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/RangeRules.cpp" line="24"/>
        <source>Это range поддерживает только относительное изменение.</source>
        <translation>This range only supports relative changes.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/ModeRules.cpp" line="10"/>
        <source>Для mode требуются непустые экземпляр и значение.</source>
        <translation>Mode requires a nonempty instance and value.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/ModeRules.cpp" line="19"/>
        <location filename="../src/iot/core/ColorRules.cpp" line="65"/>
        <source>Устройство не поддерживает такой режим или сцену.</source>
        <translation>The device does not support this mode or scene.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/ColorRules.cpp" line="9"/>
        <source>Цвет: rgb — целое 0–16777215; temperature_k — положительное целое; также доступны hsv и scene.</source>
        <translation>Color: rgb must be an integer from 0–16777215; temperature_k must be a positive integer; hsv and scene are also available.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/ColorRules.cpp" line="25"/>
        <source>Режим или сцена не должны быть пустыми.</source>
        <translation>The mode or scene must not be empty.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/ColorRules.cpp" line="35"/>
        <source>HSV должен быть JSON-объектом с h, s и v.</source>
        <translation>HSV must be a JSON object with h, s, and v.</translation>
    </message>
    <message>
        <location filename="../src/iot/core/ColorRules.cpp" line="39"/>
        <source>HSV: h должен быть 0–360; s и v — 0–100.</source>
        <translation>HSV: h must be 0–360; s and v must be 0–100.</translation>
    </message>
</context>
<context>
    <name>CliApp</name>
    <message>
        <location filename="../src/app/CliApp.cpp" line="94"/>
        <location filename="../src/app/CliApp.cpp" line="117"/>
        <source>Время выполнения команды истекло.</source>
        <translation>The command timed out.</translation>
    </message>
    <message>
        <location filename="../src/app/CliApp.cpp" line="106"/>
        <source>Сначала войдите в аккаунт через приложение.</source>
        <translation>Sign in through the desktop application first.</translation>
    </message>
    <message>
        <location filename="../src/app/CliApp.cpp" line="110"/>
        <source>Не удалось прочитать сохранённые данные входа.</source>
        <translation>Could not read the saved sign-in data.</translation>
    </message>
    <message>
        <location filename="../src/app/CliApp.cpp" line="114"/>
        <source>Доступ к сохранённым данным входа отменён.</source>
        <translation>Access to the saved sign-in data was canceled.</translation>
    </message>
</context>
<context>
    <name>CliArguments</name>
    <message>
        <location filename="../src/cli/CliArguments.cpp" line="8"/>
        <location filename="../src/cli/CliArguments.cpp" line="13"/>
        <source>Укажите ровно один непустой --id или --name.</source>
        <translation>Specify exactly one nonempty --id or --name.</translation>
    </message>
    <message>
        <location filename="../src/cli/CliArguments.cpp" line="17"/>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="31"/>
        <source>ID дома не должен быть пустым.</source>
        <translation>The household ID must not be empty.</translation>
    </message>
    <message>
        <location filename="../src/cli/CliArguments.cpp" line="20"/>
        <source>--household применяется к списку или поиску по имени, а не к --id.</source>
        <translation>--household applies to lists or name lookup; it cannot be combined with --id.</translation>
    </message>
    <message>
        <location filename="../src/cli/CliArguments.cpp" line="26"/>
        <source>Точный ID устройства или сценария.</source>
        <translation>Exact device or scenario ID.</translation>
    </message>
    <message>
        <location filename="../src/cli/CliArguments.cpp" line="27"/>
        <source>Точное имя; неоднозначные имена отклоняются.</source>
        <translation>Exact name; ambiguous names are rejected.</translation>
    </message>
</context>
<context>
    <name>CliCommand</name>
    <message>
        <location filename="../src/cli/CliCommand.cpp" line="12"/>
        <source>Повторяющийся параметр: --%1.</source>
        <translation>Repeated option: --%1.</translation>
    </message>
    <message>
        <location filename="../src/cli/CliCommand.cpp" line="25"/>
        <source>--timeout должен быть от 1 до 3600000 миллисекунд.</source>
        <translation>--timeout must be between 1 and 3600000 milliseconds.</translation>
    </message>
</context>
<context>
    <name>CliRunner</name>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="35"/>
        <source>Ошибка [%1]: %2
</source>
        <translation>Error [%1]: %2
</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="68"/>
        <source>Время выполнения команды истекло.</source>
        <translation>The command timed out.</translation>
    </message>
</context>
<context>
    <name>CliValueParsers</name>
    <message>
        <location filename="../src/cli/capabilities/ValueParsers.cpp" line="12"/>
        <source>Логическое значение должно быть on/off или true/false.</source>
        <translation>A boolean value must be on/off or true/false.</translation>
    </message>
    <message>
        <location filename="../src/cli/capabilities/ValueParsers.cpp" line="20"/>
        <source>Значение должно быть конечным числом.</source>
        <translation>The value must be a finite number.</translation>
    </message>
    <message>
        <location filename="../src/cli/capabilities/ValueParsers.cpp" line="36"/>
        <source>HSV должен быть JSON-объектом с h, s и v.</source>
        <translation>HSV must be a JSON object with h, s, and v.</translation>
    </message>
</context>
<context>
    <name>ColorSetting</name>
    <message>
        <location filename="../src/qml/controls/ColorSetting.qml" line="34"/>
        <source>Цвета</source>
        <translation>Colors</translation>
    </message>
    <message>
        <location filename="../src/qml/controls/ColorSetting.qml" line="35"/>
        <source>Режимы</source>
        <translation>Modes</translation>
    </message>
</context>
<context>
    <name>CommandRegistry</name>
    <message>
        <location filename="../src/cli/CommandRegistry.cpp" line="25"/>
        <source>Управление устройствами и сценариями Яндекс Дома.</source>
        <translation>Control Yandex Home devices and scenarios.</translation>
    </message>
    <message>
        <location filename="../src/cli/CommandRegistry.cpp" line="27"/>
        <source>Команда из списка ниже.</source>
        <translation>A command from the list below.</translation>
    </message>
    <message>
        <location filename="../src/cli/CommandRegistry.cpp" line="28"/>
        <source>JSON в stdout; ошибки JSON в stderr.</source>
        <translation>JSON to stdout; JSON errors to stderr.</translation>
    </message>
    <message>
        <location filename="../src/cli/CommandRegistry.cpp" line="29"/>
        <source>Общий таймаут в миллисекундах (по умолчанию 30000).</source>
        <translation>Total timeout in milliseconds (default: 30000).</translation>
    </message>
    <message>
        <location filename="../src/cli/CommandRegistry.cpp" line="33"/>
        <source>Команды:</source>
        <translation>Commands:</translation>
    </message>
    <message>
        <location filename="../src/cli/CommandRegistry.cpp" line="51"/>
        <source>Укажите только одну команду.</source>
        <translation>Specify only one command.</translation>
    </message>
    <message>
        <location filename="../src/cli/CommandRegistry.cpp" line="59"/>
        <source>Параметр --%1 несовместим со старой командой.</source>
        <translation>Option --%1 conflicts with the legacy command.</translation>
    </message>
    <message>
        <location filename="../src/cli/CommandRegistry.cpp" line="96"/>
        <source>Неизвестная команда. Используйте --help.</source>
        <translation>Unknown command. Use --help.</translation>
    </message>
    <message>
        <location filename="../src/cli/CommandRegistry.cpp" line="82"/>
        <source>Недопустимый параметр для команды: --%1.</source>
        <translation>Invalid option for this command: --%1.</translation>
    </message>
</context>
<context>
    <name>DataColorModes</name>
    <message>
        <location filename="DataStrings.cpp" line="34"/>
        <source>Северное Сияние</source>
        <translation>Northern Lights</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="35"/>
        <source>Рождество</source>
        <translation>Christmas</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="36"/>
        <source>Сказочные огни</source>
        <translation>Fairy Lights</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="37"/>
        <source>Год змеи</source>
        <translation>Year of the Snake</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="38"/>
        <source>Алиса</source>
        <translation>Alice</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="39"/>
        <source>Вечеринка</source>
        <translation>Party</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="40"/>
        <source>Джунгли</source>
        <translation>Jungle</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="41"/>
        <source>Неон</source>
        <translation>Neon</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="42"/>
        <source>Ночь</source>
        <translation>Night</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="43"/>
        <source>Океан</source>
        <translation>Ocean</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="44"/>
        <source>Романтика</source>
        <translation>Romance</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="45"/>
        <source>Свеча</source>
        <translation>Candle</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="46"/>
        <source>Сирена</source>
        <translation>Siren</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="47"/>
        <source>Тревога</source>
        <translation>Alarm</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="48"/>
        <source>Фантазия</source>
        <translation>Fantasy</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="49"/>
        <source>Чтение</source>
        <translation>Reading</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="50"/>
        <source>Ужин</source>
        <translation>Dinner</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="51"/>
        <source>Гирлядна</source>
        <translation>Garland</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="52"/>
        <source>Кино</source>
        <translation>Movie</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="53"/>
        <source>Отдых</source>
        <translation>Relax</translation>
    </message>
</context>
<context>
    <name>DataColors</name>
    <message>
        <location filename="DataStrings.cpp" line="6"/>
        <source>Красный</source>
        <translation>Red</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="7"/>
        <source>Коралловый</source>
        <translation>Coral</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="8"/>
        <source>Оранжевый</source>
        <translation>Orange</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="9"/>
        <source>Желтый</source>
        <translation>Yellow</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="10"/>
        <source>Салатовый</source>
        <translation>Lime</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="11"/>
        <source>Зеленый</source>
        <translation>Green</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="12"/>
        <source>Изумрудный</source>
        <translation>Emerald</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="13"/>
        <source>Бирюзовый</source>
        <translation>Turquoise</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="14"/>
        <source>Голубой</source>
        <translation>Sky blue</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="15"/>
        <source>Синий</source>
        <translation>Blue</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="16"/>
        <source>Лунный</source>
        <translation>Moonlight</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="17"/>
        <source>Сиреневый</source>
        <translation>Lilac</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="18"/>
        <source>Фиолетовый</source>
        <translation>Violet</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="19"/>
        <source>Пурпурный</source>
        <translation>Purple</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="20"/>
        <source>Розовый</source>
        <translation>Pink</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="21"/>
        <source>Малиновый</source>
        <translation>Raspberry</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="22"/>
        <source>Лиловый</source>
        <translation>Mauve</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="23"/>
        <source>Огненный белый</source>
        <translation>Flame white</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="24"/>
        <source>Мягкий белый</source>
        <translation>Soft white</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="25"/>
        <source>Теплый белый</source>
        <translation>Warm white</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="26"/>
        <source>Белый</source>
        <translation>White</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="27"/>
        <source>Дневной белый</source>
        <translation>Daylight white</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="28"/>
        <source>Холодный белый</source>
        <translation>Cool white</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="29"/>
        <source>Туманный белый</source>
        <translation>Mist white</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="30"/>
        <source>Небесный белый</source>
        <translation>Sky white</translation>
    </message>
</context>
<context>
    <name>DataErrors</name>
    <message>
        <location filename="DataStrings.cpp" line="228"/>
        <source>Открыта дверца</source>
        <translation>Door is open</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="229"/>
        <source>Не забудьте закрыть дверцу.</source>
        <translation>Close the door.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="230"/>
        <source>Открыта крышка</source>
        <translation>Lid is open</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="231"/>
        <source>Не забудьте закрыть крышку.</source>
        <translation>Close the lid.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="232"/>
        <source>Удаленное управление устройством отключено</source>
        <translation>Remote control is disabled</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="233"/>
        <source>Сначала нужно спросить разрешения у самого устройства: проверьте, на нём должна быть специальная кнопка.</source>
        <translation>Enable remote control on the device. Check for a dedicated button.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="234"/>
        <source>Недостаточно воды</source>
        <translation>Not enough water</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="235"/>
        <source>Попробуйте долить воды.</source>
        <translation>Add more water.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="236"/>
        <source>Низкий уровень заряда</source>
        <translation>Low battery</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="237"/>
        <source>Устройство нужно зарядить.</source>
        <translation>Charge the device.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="238"/>
        <source>Контейнер полон</source>
        <translation>Container is full</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="239"/>
        <source>Сначала нужно очистить контейнер.</source>
        <translation>Empty the container first.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="240"/>
        <source>Контейнер пуст</source>
        <translation>Container is empty</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="241"/>
        <source>В контейнер нужно что-нибудь положить.</source>
        <translation>Put something in the container.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="242"/>
        <source>Сливной поддон полон</source>
        <translation>Drain tray is full</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="243"/>
        <source>Нужно очистить поддон.</source>
        <translation>Empty the tray.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="244"/>
        <source>Устройство застряло</source>
        <translation>Device is stuck</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="245"/>
        <source>Кажется, на пути препятствие, его нужно убрать.</source>
        <translation>Remove the obstacle blocking the device.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="246"/>
        <source>Устройство выключено</source>
        <translation>Device is off</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="247"/>
        <source>Сначала нужно включить устройство.</source>
        <translation>Turn on the device first.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="248"/>
        <source>Прошивка устарела</source>
        <translation>Firmware is outdated</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="249"/>
        <source>Нужно обновить прошивку устройства, которым хотите управлять.</source>
        <translation>Update the device firmware.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="250"/>
        <source>Недостаточно моющего средства</source>
        <translation>Not enough detergent</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="251"/>
        <source>Добавьте моющее средство.</source>
        <translation>Add detergent.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="252"/>
        <source>Требуется вмешательство человека</source>
        <translation>Manual intervention required</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="253"/>
        <source>Что-то не так с устройством: пожалуйста, осмотрите его.</source>
        <translation>Something is wrong with the device. Please check it.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="254"/>
        <source>Устройство недоступно</source>
        <translation>Device is unavailable</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="255"/>
        <source>Устройство не отвечает. Проверьте, вдруг оно выключено или пропал интернет.</source>
        <translation>The device is not responding. Check its power and internet connection.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="256"/>
        <source>Устройство занято</source>
        <translation>Device is busy</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="257"/>
        <source>Устройство уже работает. Подождите, пока оно закончит.</source>
        <translation>The device is already running. Wait for it to finish.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="258"/>
        <source>Неизвестная внутренняя ошибка</source>
        <translation>Unknown internal error</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="259"/>
        <source>Случилось что-то непонятное. Подождите немного и попробуйте ещё раз.</source>
        <translation>An unexpected error occurred. Wait a moment and try again.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="260"/>
        <source>Недопустимое действие</source>
        <translation>Invalid action</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="261"/>
        <source>Это устройство так не умеет. Попробуйте что-нибудь другое.</source>
        <translation>This device does not support that action. Try something else.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="262"/>
        <source>Недопустимое значение</source>
        <translation>Invalid value</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="263"/>
        <source>Какое-то незнакомое значение. Попробуйте другое.</source>
        <translation>This value is not recognized. Try another one.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="264"/>
        <source>Не поддерживается в текущем режиме работы устройства</source>
        <translation>Unsupported in the current device mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="265"/>
        <source>В этом режиме такая команда не работает.</source>
        <translation>That command does not work in this mode.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="266"/>
        <source>Ошибка в OAuth2 токене пользователя</source>
        <translation>User OAuth2 token error</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="267"/>
        <source>Попробуйте привязать устройство заново, а то оно отвязалось.</source>
        <translation>Try linking the device again.</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="268"/>
        <source>Устройство не найдено</source>
        <translation>Device not found</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="269"/>
        <source>Данное устройство вам не принадлежит.</source>
        <translation>This device does not belong to your account.</translation>
    </message>
</context>
<context>
    <name>DataEvents</name>
    <message>
        <location filename="DataStrings.cpp" line="190"/>
        <source>переворачивание</source>
        <translation>tilt</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="191"/>
        <source>падение</source>
        <translation>fall</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="192"/>
        <source>вибрация</source>
        <translation>vibration</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="193"/>
        <source>открыто</source>
        <translation>open</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="194"/>
        <source>закрыто</source>
        <translation>closed</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="195"/>
        <source>одиночное нажатие</source>
        <translation>single press</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="196"/>
        <source>двойное нажатие</source>
        <translation>double press</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="197"/>
        <source>долгое нажатие</source>
        <translation>long press</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="198"/>
        <source>обнаружено</source>
        <translation>detected</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="199"/>
        <source>не обнаружено</source>
        <translation>not detected</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="200"/>
        <source>высокий уровень</source>
        <translation>high level</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="201"/>
        <source>низкий</source>
        <translation>low</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="202"/>
        <source>нормальный</source>
        <translation>normal</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="203"/>
        <source>пустой</source>
        <translation>empty</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="204"/>
        <source>нет протечки</source>
        <translation>no leak</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="205"/>
        <source>протечка</source>
        <translation>leak</translation>
    </message>
</context>
<context>
    <name>DataInstances</name>
    <message>
        <location filename="DataStrings.cpp" line="137"/>
        <source>Вкл/Выкл</source>
        <translation>On/Off</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="138"/>
        <source>Яркость</source>
        <translation>Brightness</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="139"/>
        <source>Канал</source>
        <translation>Channel</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="140"/>
        <source>Влажность</source>
        <translation>Humidity</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="141"/>
        <source>Открытие</source>
        <translation>Opening</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="142"/>
        <source>Температура</source>
        <translation>Temperature</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="143"/>
        <source>Громкость</source>
        <translation>Volume</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="144"/>
        <source>Подсветка</source>
        <translation>Backlight</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="145"/>
        <source>Детский режим</source>
        <translation>Child lock</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="146"/>
        <source>Ионизация</source>
        <translation>Ionization</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="147"/>
        <source>Поддержание тепла</source>
        <translation>Keep warm</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="148"/>
        <source>Выключение звука</source>
        <translation>Mute</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="149"/>
        <source>Вращение</source>
        <translation>Oscillation</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="150"/>
        <source>Пауза</source>
        <translation>Pause</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="151"/>
        <source>Режим уборки</source>
        <translation>Cleaning mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="152"/>
        <source>Режим работы кофеварки</source>
        <translation>Coffee maker mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="153"/>
        <source>Режим мытья посуды</source>
        <translation>Dishwashing mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="154"/>
        <source>Режима работы скорости вентиляции</source>
        <translation>Fan speed mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="155"/>
        <source>Режим нагрева</source>
        <translation>Heating mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="156"/>
        <source>Источник сигнала</source>
        <translation>Input source</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="157"/>
        <source>Программа работы</source>
        <translation>Program</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="158"/>
        <source>Направление воздуха</source>
        <translation>Airflow direction</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="159"/>
        <source>Режима приготовления чая</source>
        <translation>Tea brewing mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="160"/>
        <source>Температурный режим</source>
        <translation>Thermostat mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="161"/>
        <source>Режима вентиляции</source>
        <translation>Ventilation mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="162"/>
        <source>Скорость работы</source>
        <translation>Operating speed</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="163"/>
        <source>Потребление тока</source>
        <translation>Current draw</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="164"/>
        <source>Уровень заряда</source>
        <translation>Battery level</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="165"/>
        <source>Уровень углекислого газа</source>
        <translation>CO2 level</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="166"/>
        <source>Счетчик электроэнергии</source>
        <translation>Electricity meter</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="167"/>
        <source>Уровень корма</source>
        <translation>Food level</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="168"/>
        <source>Счетчик газа</source>
        <translation>Gas meter</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="169"/>
        <source>Счетчик тепла</source>
        <translation>Heat meter</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="170"/>
        <source>Освещение</source>
        <translation>Illumination</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="171"/>
        <source>Счетчик</source>
        <translation>Meter</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="172"/>
        <source>Загрязнение воздуха PM1</source>
        <translation>PM1 air pollution</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="173"/>
        <source>Загрязнение воздуха PM2.5</source>
        <translation>PM2.5 air pollution</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="174"/>
        <source>Загрязнение воздуха PM10</source>
        <translation>PM10 air pollution</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="175"/>
        <source>Потребление мощности</source>
        <translation>Power consumption</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="176"/>
        <source>Давление</source>
        <translation>Pressure</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="177"/>
        <source>Загрязнение органикой</source>
        <translation>VOC level</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="178"/>
        <source>Напряжение</source>
        <translation>Voltage</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="179"/>
        <source>Уровень воды</source>
        <translation>Water level</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="180"/>
        <source>Счетчик воды</source>
        <translation>Water meter</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="181"/>
        <source>Вибрация</source>
        <translation>Vibration</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="182"/>
        <source>Нажатие</source>
        <translation>Button press</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="183"/>
        <source>Движение</source>
        <translation>Motion</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="184"/>
        <source>Дым</source>
        <translation>Smoke</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="185"/>
        <source>Газ</source>
        <translation>Gas</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="186"/>
        <source>Протечка воды</source>
        <translation>Water leak</translation>
    </message>
</context>
<context>
    <name>DataModes</name>
    <message>
        <location filename="DataStrings.cpp" line="57"/>
        <source>Влажная уборка</source>
        <translation>Wet cleaning</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="58"/>
        <source>Сухая уборка</source>
        <translation>Dry cleaning</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="59"/>
        <source>Смешанная уборка</source>
        <translation>Mixed cleaning</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="60"/>
        <source>Автоматический режим</source>
        <translation>Automatic mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="61"/>
        <source>Экономичный режим</source>
        <translation>Eco mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="62"/>
        <source>Умный режим</source>
        <translation>Smart mode</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="63"/>
        <source>Турбо</source>
        <translation>Turbo</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="64"/>
        <source>Охлаждение</source>
        <translation>Cooling</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="65"/>
        <source>Режим осушения</source>
        <translation>Dehumidification</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="66"/>
        <source>Вентиляция</source>
        <translation>Ventilation</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="67"/>
        <source>Обогрев</source>
        <translation>Heating</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="68"/>
        <source>Подогрев</source>
        <translation>Warming</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="69"/>
        <source>Высокая скорость</source>
        <translation>High speed</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="70"/>
        <source>Низкая скорость</source>
        <translation>Low speed</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="71"/>
        <source>Средняя скорость</source>
        <translation>Medium speed</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="72"/>
        <source>Максимальный</source>
        <translation>Maximum</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="73"/>
        <source>Минимальный</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="74"/>
        <source>Быстрый</source>
        <translation>Fast</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="75"/>
        <source>Медленный</source>
        <translation>Slow</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="76"/>
        <source>Экспресс</source>
        <translation>Express</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="77"/>
        <source>Нормальный</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="78"/>
        <source>Тихий</source>
        <translation>Quiet</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="79"/>
        <source>Горизонтальный</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="80"/>
        <source>Неподвижный</source>
        <translation>Fixed</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="81"/>
        <source>Вертикальный</source>
        <translation>Vertical</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="82"/>
        <source>Приток воздуха</source>
        <translation>Air intake</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="83"/>
        <source>Отток воздуха</source>
        <translation>Air exhaust</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="84"/>
        <source>Первый</source>
        <translation>First</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="85"/>
        <source>Второй</source>
        <translation>Second</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="86"/>
        <source>Третий</source>
        <translation>Third</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="87"/>
        <source>Четвертый</source>
        <translation>Fourth</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="88"/>
        <source>Пятый</source>
        <translation>Fifth</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="89"/>
        <source>Шестой</source>
        <translation>Sixth</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="90"/>
        <source>Седьмой</source>
        <translation>Seventh</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="91"/>
        <source>Восьмой</source>
        <translation>Eighth</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="92"/>
        <source>Девятый</source>
        <translation>Ninth</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="93"/>
        <source>Десятый</source>
        <translation>Tenth</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="94"/>
        <source>Американо</source>
        <translation>Americano</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="95"/>
        <source>Капучино</source>
        <translation>Cappuccino</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="96"/>
        <source>Двойной эспрессо</source>
        <translation>Double espresso</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="97"/>
        <source>Эспрессо</source>
        <translation>Espresso</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="98"/>
        <source>Латте</source>
        <translation>Latte</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="99"/>
        <source>Черный чай</source>
        <translation>Black tea</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="100"/>
        <source>Цветочный чай</source>
        <translation>Floral tea</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="101"/>
        <source>Зеленый чай</source>
        <translation>Green tea</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="102"/>
        <source>Травяной чай</source>
        <translation>Herbal tea</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="103"/>
        <source>Чай улун</source>
        <translation>Oolong tea</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="104"/>
        <source>Чай пуэр</source>
        <translation>Pu-erh tea</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="105"/>
        <source>Красный чай</source>
        <translation>Red tea</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="106"/>
        <source>Белый чай</source>
        <translation>White tea</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="107"/>
        <source>Мойка стекла</source>
        <translation>Window cleaning</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="108"/>
        <source>Интенсивный</source>
        <translation>Intensive</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="109"/>
        <source>Ополаскивание</source>
        <translation>Rinsing</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="110"/>
        <source>Холодец</source>
        <translation>Aspic</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="111"/>
        <source>Детское питание</source>
        <translation>Baby food</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="112"/>
        <source>Выпечка</source>
        <translation>Baking</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="113"/>
        <source>Хлеб</source>
        <translation>Bread</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="114"/>
        <source>Варка</source>
        <translation>Boiling</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="115"/>
        <source>Крупы</source>
        <translation>Grains</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="116"/>
        <source>Чизкейк</source>
        <translation>Cheesecake</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="117"/>
        <source>Фритюр</source>
        <translation>Deep frying</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="118"/>
        <source>Десерты</source>
        <translation>Desserts</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="119"/>
        <source>Дичь</source>
        <translation>Game</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="120"/>
        <source>Жарка</source>
        <translation>Frying</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="121"/>
        <source>Макароны</source>
        <translation>Macaroni</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="122"/>
        <source>Молочная каша</source>
        <translation>Milk porridge</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="123"/>
        <source>Мультиповар</source>
        <translation>Multi-cook</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="124"/>
        <source>Паста</source>
        <translation>Pasta</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="125"/>
        <source>Плов</source>
        <translation>Pilaf</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="126"/>
        <source>Пицца</source>
        <translation>Pizza</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="127"/>
        <source>Соус</source>
        <translation>Sauce</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="128"/>
        <source>Томление</source>
        <translation>Slow cooking</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="129"/>
        <source>Суп</source>
        <translation>Soup</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="130"/>
        <source>Пар</source>
        <translation>Steam</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="131"/>
        <source>Тушение</source>
        <translation>Stewing</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="132"/>
        <source>Вакуум</source>
        <translation>Vacuum</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="133"/>
        <source>Йогурт</source>
        <translation>Yogurt</translation>
    </message>
</context>
<context>
    <name>DataUnits</name>
    <message>
        <location filename="DataStrings.cpp" line="209"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="210"/>
        <source>°</source>
        <translation>°</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="211"/>
        <source>K</source>
        <translation>K</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="212"/>
        <source> Ам</source>
        <translation> A</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="213"/>
        <source> ппм</source>
        <translation> ppm</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="214"/>
        <source> кВт⋅ч</source>
        <translation> kWh</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="215"/>
        <source> м³</source>
        <translation> m³</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="216"/>
        <source> Гкал</source>
        <translation> Gcal</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="217"/>
        <source> Люкс</source>
        <translation> lx</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="218"/>
        <source> мкг/м3</source>
        <translation> µg/m³</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="219"/>
        <source>Ватт</source>
        <translation> W</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="220"/>
        <source> атм</source>
        <translation> atm</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="221"/>
        <source> па</source>
        <translation> Pa</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="222"/>
        <source> бар</source>
        <translation> bar</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="223"/>
        <source> мм рт. ст</source>
        <translation> mmHg</translation>
    </message>
    <message>
        <location filename="DataStrings.cpp" line="224"/>
        <source> V</source>
        <translation> V</translation>
    </message>
</context>
<context>
    <name>DeviceCommands</name>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="15"/>
        <source>Дом с таким ID не найден.</source>
        <translation>No household with this ID was found.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="41"/>
        <source>ID	Имя	Тип	Дом
</source>
        <translation>ID	Name	Type	Household
</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="69"/>
        <source>Устройство с таким именем не найдено.</source>
        <translation>No device with this name was found.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="71"/>
        <source>Найдено несколько устройств. Укажите --id или --household.</source>
        <translation>Several devices matched. Specify --id or --household.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="81"/>
        <source>Ответ содержит другое устройство.</source>
        <translation>The response contains a different device.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="122"/>
        <source>Устройство не поддерживает это умение и экземпляр.</source>
        <translation>The device does not support this capability and instance.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="126"/>
        <source>Устройство содержит несколько одинаковых умений.</source>
        <translation>The device contains several matching capabilities.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="135"/>
        <source>Команда для устройства %1 выполнена.
</source>
        <translation>Command for device %1 completed.
</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="144"/>
        <source>ID дома для списка или поиска устройства по имени.</source>
        <translation>Household ID for listing or device name lookup.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="145"/>
        <source>on_off, range, mode, toggle или color_setting.</source>
        <translation>on_off, range, mode, toggle, or color_setting.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="146"/>
        <source>Экземпляр умения: brightness, mute, rgb и т. п.</source>
        <translation>Capability instance: brightness, mute, rgb, etc.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="147"/>
        <source>Значение: on/off, число, режим, цвет или JSON HSV.</source>
        <translation>Value: on/off, a number, mode, color, or HSV JSON.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="148"/>
        <source>Относительное изменение range вместо абсолютного значения.</source>
        <translation>Apply a relative range change instead of an absolute value.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="149"/>
        <source>Совместимость: devices list.</source>
        <translation>Compatibility: devices list.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="150"/>
        <source>Совместимость: on/off по имени устройства.</source>
        <translation>Compatibility: on/off by device name.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="151"/>
        <source>Справка для старого --on_off.</source>
        <translation>Help for the legacy --on_off option.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="152"/>
        <source>Список устройств с ID.</source>
        <translation>List devices with their IDs.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="154"/>
        <source>Состояние, умения и свойства устройства.</source>
        <translation>Show device state, capabilities, and properties.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/DeviceCommands.cpp" line="156"/>
        <source>Отправить действие устройству.</source>
        <translation>Send an action to a device.</translation>
    </message>
</context>
<context>
    <name>DeviceHeader</name>
    <message>
        <location filename="../src/qml/components/DeviceHeader.qml" line="70"/>
        <source>Устройство оффлайн!</source>
        <translation>Device is offline!</translation>
    </message>
</context>
<context>
    <name>DevicePage</name>
    <message>
        <location filename="../src/qml/pages/DevicePage.qml" line="13"/>
        <source>Ошибка</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/DevicePage.qml" line="24"/>
        <source>Произошла ошибка!</source>
        <translation>An error occurred!</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/DevicePage.qml" line="62"/>
        <source>Нет связи с устройством</source>
        <translation>No connection to the device</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/DevicePage.qml" line="63"/>
        <source>Попробовать снова</source>
        <translation>Try again</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/DevicePage.qml" line="72"/>
        <source>Умения</source>
        <translation>Capabilities</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/DevicePage.qml" line="73"/>
        <source>Свойства</source>
        <translation>Properties</translation>
    </message>
</context>
<context>
    <name>DevicesPage</name>
    <message>
        <location filename="../src/qml/pages/DevicesPage.qml" line="16"/>
        <source>Комнаты</source>
        <translation>Rooms</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/DevicesPage.qml" line="38"/>
        <source>Что-то пошло не так!</source>
        <translation>Something went wrong!</translation>
    </message>
</context>
<context>
    <name>ErrorDialog</name>
    <message>
        <location filename="../src/qml/ui/ErrorDialog.qml" line="73"/>
        <source>Ок</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>ErrorPage</name>
    <message>
        <location filename="../src/qml/pages/ErrorPage.qml" line="7"/>
        <source>Что-то пошло не так! %1</source>
        <translation>Something went wrong! %1</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/ErrorPage.qml" line="8"/>
        <source>Попробовать ещё раз</source>
        <translation>Try again</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/ErrorPage.qml" line="9"/>
        <source>Выйти из аккаунта</source>
        <translation>Sign out</translation>
    </message>
</context>
<context>
    <name>LoadingPage</name>
    <message>
        <location filename="../src/qml/pages/LoadingPage.qml" line="10"/>
        <source>Загрузка...</source>
        <translation>Loading...</translation>
    </message>
</context>
<context>
    <name>LocalCommands</name>
    <message>
        <location filename="../src/cli/commands/LocalCommands.cpp" line="11"/>
        <source>Имя: %1
Email: %2
</source>
        <translation>Name: %1
Email: %2
</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/LocalCommands.cpp" line="27"/>
        <source>Настройки сброшены; выход из аккаунта выполнен.
</source>
        <translation>Settings reset; signed out.
</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/LocalCommands.cpp" line="36"/>
        <source>Подтверждение сброса настроек и выхода из аккаунта.</source>
        <translation>Confirm resetting settings and signing out.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/LocalCommands.cpp" line="37"/>
        <source>Совместимость: account show.</source>
        <translation>Compatibility: account show.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/LocalCommands.cpp" line="38"/>
        <source>Совместимость: reset.</source>
        <translation>Compatibility: reset.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/LocalCommands.cpp" line="39"/>
        <source>Имя и email аккаунта.</source>
        <translation>Show the account name and email.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/LocalCommands.cpp" line="41"/>
        <source>Сбросить настройки и выйти из аккаунта.</source>
        <translation>Reset settings and sign out.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/LocalCommands.cpp" line="19"/>
        <source>Для сброса укажите --i-know-what-i-am-doing.</source>
        <translation>To reset, specify --i-know-what-i-am-doing.</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <location filename="../src/qml/Main.qml" line="15"/>
        <source>Yandex Home Desktop</source>
        <translation>Yandex Home Desktop</translation>
    </message>
    <message>
        <source>Кадр/с: %1</source>
        <translation type="vanished">FPS: %1</translation>
    </message>
</context>
<context>
    <name>MainPage</name>
    <message>
        <location filename="../src/qml/pages/MainPage.qml" line="33"/>
        <source>Выберите Дом</source>
        <translation>Select a home</translation>
    </message>
</context>
<context>
    <name>MyTab</name>
    <message>
        <location filename="../src/qml/ui/MyTab.qml" line="9"/>
        <source>Вкладка</source>
        <translation>Tab</translation>
    </message>
</context>
<context>
    <name>OnOff</name>
    <message>
        <location filename="../src/qml/controls/OnOff.qml" line="65"/>
        <source>Вкл</source>
        <translation>On</translation>
    </message>
    <message>
        <location filename="../src/qml/controls/OnOff.qml" line="110"/>
        <source>Выкл</source>
        <translation>Off</translation>
    </message>
</context>
<context>
    <name>PropertiesModel</name>
    <message>
        <location filename="../src/models/DeviceModel/PropertiesModel.cpp" line="62"/>
        <source>Неизвестный тип свойства</source>
        <translation>Unknown property type</translation>
    </message>
</context>
<context>
    <name>RoomDevicesList</name>
    <message>
        <location filename="../src/qml/components/RoomDevicesList.qml" line="31"/>
        <source>В этой комнате нет устройств!</source>
        <translation>No devices in this room!</translation>
    </message>
</context>
<context>
    <name>ScenarioCommands</name>
    <message>
        <location filename="../src/cli/commands/ScenarioCommands.cpp" line="13"/>
        <source>ID	Имя	Активен
</source>
        <translation>ID	Name	Active
</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/ScenarioCommands.cpp" line="39"/>
        <source>Сценарий не найден.</source>
        <translation>Scenario not found.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/ScenarioCommands.cpp" line="40"/>
        <source>Найдено несколько сценариев. Укажите --id.</source>
        <translation>Several scenarios matched. Specify --id.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/ScenarioCommands.cpp" line="42"/>
        <source>Сценарий неактивен.</source>
        <translation>The scenario is inactive.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/ScenarioCommands.cpp" line="46"/>
        <source>Сценарий %1 выполнен.
</source>
        <translation>Scenario %1 completed.
</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/ScenarioCommands.cpp" line="56"/>
        <source>Список сценариев с ID.</source>
        <translation>List scenarios with their IDs.</translation>
    </message>
    <message>
        <location filename="../src/cli/commands/ScenarioCommands.cpp" line="58"/>
        <source>Выполнить активный сценарий.</source>
        <translation>Run an active scenario.</translation>
    </message>
</context>
<context>
    <name>ScenariosPage</name>
    <message>
        <location filename="../src/qml/pages/ScenariosPage.qml" line="17"/>
        <source>Ошибка</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/ScenariosPage.qml" line="18"/>
        <source>Не удалось выполнить сценарий</source>
        <translation>Could not run the scenario</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/ScenariosPage.qml" line="33"/>
        <source>Все сценарии</source>
        <translation>All scenarios</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/ScenariosPage.qml" line="62"/>
        <source>Что-то пошло не так!</source>
        <translation>Something went wrong!</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/ScenariosPage.qml" line="67"/>
        <source>Пока что у вас нет сценариев</source>
        <translation>No scenarios yet</translation>
    </message>
</context>
<context>
    <name>ScenariosViewModel</name>
    <message>
        <location filename="../src/models/ScenariosModel/ScenariosViewModel.cpp" line="14"/>
        <source>Не удалось выполнить сценарий</source>
        <translation>Could not run the scenario</translation>
    </message>
</context>
<context>
    <name>SettingsPage</name>
    <message>
        <location filename="../src/qml/pages/SettingsPage.qml" line="12"/>
        <source>Настройки</source>
        <translation>Settings</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/SettingsPage.qml" line="21"/>
        <source>Tray-режим</source>
        <translation>Tray mode</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/SettingsPage.qml" line="22"/>
        <source>Приложение будет отображаться
как иконка на панели задач</source>
        <translation>The app will appear as an icon
in the system tray</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/SettingsPage.qml" line="24"/>
        <source>Тема</source>
        <translation>Theme</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/SettingsPage.qml" line="25"/>
        <source>Светлая</source>
        <translation>Light</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/SettingsPage.qml" line="25"/>
        <source>Тёмная</source>
        <translation>Dark</translation>
    </message>
    <message>
        <location filename="../src/qml/pages/SettingsPage.qml" line="27"/>
        <source>GitHub</source>
        <translation>GitHub</translation>
    </message>
</context>
<context>
    <name>TopBar</name>
    <message>
        <location filename="../src/qml/components/TopBar.qml" line="89"/>
        <source>Устройства</source>
        <translation>Devices</translation>
    </message>
    <message>
        <location filename="../src/qml/components/TopBar.qml" line="96"/>
        <source>Сценарии</source>
        <translation>Scenarios</translation>
    </message>
    <message>
        <location filename="../src/qml/components/TopBar.qml" line="103"/>
        <source>Настройки</source>
        <translation>Settings</translation>
    </message>
</context>
<context>
    <name>Unsupported</name>
    <message>
        <location filename="../src/qml/controls/Unsupported.qml" line="17"/>
        <source>Неподдерживаемое умение: %1</source>
        <translation>Unsupported capability: %1</translation>
    </message>
</context>
</TS>
