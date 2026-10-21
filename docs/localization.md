# Localization

The app chooses the first supported language in the system UI language preferences at startup, falling back to English. Russian text in the source is used for Russian, while `translations/YandexHomeDesktop_en.ts` supplies English. There is no language setting in the UI.

QML display strings use `qsTr()`, and C++ display strings use `tr()`. Keep placeholders such as `%1` inside the translatable string so another language can change word order. The translation is compiled with `lrelease` and embedded at `:/i18n/YandexHomeDesktop_en.qm` by `qt_add_translations()` in the root `CMakeLists.txt`.

Device capability names, modes, colors, property values, units, and error descriptions come from `resources/data/*.json`. Their original text stays in those files. C++ translates the display values with the appropriate `Data...` context when returning them to QML. `translations/DataStrings.cpp` exposes those JSON strings to Qt Linguist; it is scanned by `lupdate` and is not compiled into the app.

When changing display text:

1. Write new source text in Russian and wrap QML text in `qsTr()` or C++ display text in `tr()`.
2. If a bundled JSON display value changed, run `python3 scripts/update-data-translation-markers.py`.
3. Run `cmake --build <build-dir> --target update_translations`, translate any new or changed messages in `translations/YandexHomeDesktop_en.ts`, and rebuild the app. The build generates the `.qm` file automatically.

CI runs `python3 scripts/update-data-translation-markers.py --check` and `python3 scripts/check-translations.py` to ensure the committed marker file and English catalog are complete. The one-off script used to first fill the English catalog is not part of the build; the committed `.ts` file is the source of truth.

Room names, device names, account information, and other values supplied by the user or API are shown as provided. They are not entries in the translation catalog.
