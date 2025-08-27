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
| Build Status         | Branch |
|---------------------|---------|
| [![Main Windows build](https://github.com/Yagodnik/YandexHomeDesktop/actions/workflows/build-windows.yml/badge.svg?branch=main)](https://github.com/Yagodnik/YandexHomeDesktop/actions/workflows/build-windows.yml) | main |
| [![Dev Windows build](https://github.com/Yagodnik/YandexHomeDesktop/actions/workflows/build-windows.yml/badge.svg?branch=dev)](https://github.com/Yagodnik/YandexHomeDesktop/actions/workflows/build-windows.yml) | dev |

Если вы очень хотите собрать проект самостоятельно:

Создайте файл ```cmake/qt6-config.cmake``` и поместите туда вот это:
```
set(
    CMAKE_PREFIX_PATH
    "(тут будет ваш пусть до qt)/(версия, желательно 6.9)/lib/cmake"
    CACHE STRING "Qt installation path"
)

```

Ну и стоит понимать, что для полноценной работы необходимо будет сгенерировать необходимые токены.
Сгенерировать необходимые файлы можно с помощью скрипта (см workflow для Windows, этап сборки ```Generate secrets.json```)
Однако сам client id вам нужно будет сделать самостоятельно. Вот как это сделать:
1) Создайте веб приложение в https://oauth.yandex.ru/client/new
2) Укажите redirect-url: http://127.0.0.1:1337

Заполняете всё остальное и получаете все необходимые данные, копируете client_id.

### Windows
Следуйте пунктам из workflow, вероятно там будет самый актуальный способ сборки. 
Советую использовать Qt версии 6.9 и MinGW 13.10, я пробовал собирать на компиляторах старых версий, но там возникают проблемы из-за C++23.

### MacOS
Установите Qt 6.9, разместите необходимые библиотеки аналогично с workflow для Windows. Если хотите universal build, то 
нужно будет собрать qtkeychain так же universal build. 
То как примерно должна выглядит папка с либами:
```
find libs -maxdepth 2 -print 
libs
libs/qtkeychain
libs/qtkeychain/include/...
libs/qtkeychain/lib/libqt6keychain.dylib
libs/boost
libs/boost/hana.hpp
libs/boost/hana/...
```

Дальше можете воспользоваться специальным скриптом ```scripts/macos-build.sh``` из корневой папки проекта

### Linux
А под линукс может быть потом билд сделаю =)
В целом работать наверное будет, но tray режим будет требовать доработки, ~~а ещё возможно его нужно будет подгонять под каждое DE отдельно~~

# Credits
Список изображений, которые я использовал: 
1) Картинки с https://yandex.ru/quasar/
2) https://www.svgrepo.com/svg/526106/play
3) https://www.svgrepo.com/svg/507358/logout
4) https://www.svgrepo.com/svg/520909/reload
5) https://www.svgrepo.com/svg/500472/back
6) https://www.svgrepo.com/svg/521486/arrow-up
7) https://www.svgrepo.com/svg/458827/on-button