# Yandex Home Desktop
Приложение для управление устройствами умного дома яндекс с компьютера.

Windows            |  MacOS
:-------------------------:|:-------------------------:
![Windows Demo](assets/windows_demo.jpg) | ![MacOS Demo](assets/macos_demo.png)

# Tray режим
Одно из нововведений этой версии - tray режим, теперь приложение может выполнять ещё и функцию desktop приложения, но при желании может быть использовано из системного трея

# Поддерживаемые устройства
В отличии от [предыдущей версии](https://github.com/Yagodnik/YandexHomeWidgets) теперь поддерживаются все устройства, умения (исключением является умение ```devices.capabilities.video_stream```) и свойства. Однако, далеко не все из них протестированны полноценно.

# Сборка

Для сборки на macOS и Windows нужны CMake 3.28+, Ninja, Qt 6.9+ (Quick, NetworkAuth, Qt5Compat, ShaderTools и LinguistTools) и компилятор C++23. CMake загрузит закреплённые версии Boost.Hana и QtKeychain. По умолчанию в приложение включается тестовая OAuth-конфигурация `resources/auth/example.json`: приложение соберётся, но вход в аккаунт с ней не работает.

```sh
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/macos -DBUILD_DESKTOP_APP=ON
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

Для работающего входа создайте свою конфигурацию OAuth и передайте `-DYH_AUTH_CONFIG_FILE=/path/to/oauth.json` (или `-DAUTH_CONFIG_FILE=...`). Файл встраивается в приложение. Инструкции и вариант для Linux-тестов приведены в [docs/build.md](docs/build.md).

CI собирает и тестирует приложение на macOS и Windows, а на Linux запускает переносимые тесты. Сборки CI используют тестовую OAuth-конфигурацию. Для релизов используется тег `vMAJOR.MINOR.PATCH`; шаги описаны в [release workflow](.github/workflows/release.yml).

# Аргументы CLI

В этой версии я реализовал способ работы с приложением через командную строку.
Например, если у вас что-то *багнулось* с аккаунтом (а такое может быть вполне), вы можете сбросить его через CLI. 
Для этого открываете путь, куда вы установили приложение и выполняете его с аргументами: --reset --i-know-what-i-am-doing.

Примерно так: ```YandexHomeDesktop.exe --reset --i-know-what-i-am-doing``` (Для Windows)

Для MacOS ```./Applications/YandexHomeDesktop.app/Contents/MacOS/YandexHomeDesktop --reset --i-know-what-i-am-doing```

Примечание: Если вы увидите в консоли что-то такое ```Mon Sep 1 16:01:15 2025 GMT [DEBUG] AuthorizationService: Token: тут будет ваш токен```
то вы ошиблись в наборе команды и ПОЖАЛУЙСТА не присылайте мне ваш токен в комменты, если хотите что-то спросить. 
По этому токену можно управлять вашим умным домом.

Список аргументов можно посмотреть прописав ```--help```

Пример как включить устройство из CLI: ``` --on_off "имя в ковычках" --value on ```

# Credits
Список изображений, которые я использовал: 
1) Картинки с https://yandex.ru/quasar/
2) https://www.svgrepo.com/svg/526106/play
3) https://www.svgrepo.com/svg/507358/logout
4) https://www.svgrepo.com/svg/520909/reload
5) https://www.svgrepo.com/svg/500472/back
6) https://www.svgrepo.com/svg/521486/arrow-up
7) https://www.svgrepo.com/svg/458827/on-button