---
name: yh-json-serialization
description: Implement or review typed JSON serialization in YandexHomeDesktop using the repository's struct and enum macros, codecs, and file loader. Use for JSON DTOs, API payloads, and runtime configuration.
---

# YandexHomeDesktop JSON serialization

Follow the public serialization conventions in this repository. Keep JSON representation separate from validated application settings when their types differ.

## Existing examples

Read the engine and the nearest relevant caller before changing a codec:

- [SerializationTypes.h](../../../src/serialization/SerializationTypes.h) defines `JSON_STRUCT` and the codec declarations; [Serialization.h](../../../src/serialization/Serialization.h) implements conversion.
- [YandexAccountApi.cpp](../../../src/api/YandexAccountApi.cpp) shows a private `JSON_STRUCT`, document parsing, and `Serialization::From`.
- [UserInfo.h](../../../src/api/model/UserInfo.h) shows nested objects and lists; [Capabilites.h](../../../src/api/model/Capabilites.h) shows enum wrappers and dynamic maps.
- [UnitsList.cpp](../../../src/utils/UnitsList.cpp) and [JsonLoader.h](../../../src/utils/JsonLoader.h) show typed loading of bundled JSON files.
- [SerializationTests.h](../../../tests/SerializationTests.h) and [SerializationTests.cpp](../../../tests/SerializationTests.cpp) cover structs, lists, enums, and both codec directions.

These paths are relative to the installed repository skill directory, `.agents/skills/yh-json-serialization`.

## Public API

Declare JSON data with `JSON_STRUCT(Name, (Type, field), ...)`. Member names are JSON keys, including underscores; the engine has no field-alias mechanism. Use `serialization/SerializationTypes.h` in declaration-only headers and `serialization/Serialization.h` where codecs are instantiated. Private DTOs may live in an anonymous namespace, as in `YandexAccountApi.cpp`.

```cpp
JSON_STRUCT(ServerFields,
  (QString, host),
  (double, port),
  (QString, oauth_token)
);

const auto fields = Serialization::From<ServerFields>(object);
const auto encoded = Serialization::To(fields);
```

Use `JSON_ENUMERATION` for JSON string enums. It supplies a wrapper with `Type`, `Unknown`, `AsValue`, and `AsString`; unknown strings decode to `Unknown`, and `Unknown` encodes as an empty string. Validate unknown values where the caller's contract requires it.

Ordinary callers should use these macros and `From`/`To`. Keep `ISerializable`, `BOOST_HANA_DEFINE_STRUCT`, Hana accessors, and `Serialization::details` inside the engine rather than recreating its reflection machinery at call sites. Free-form JSON maps and transport envelopes can remain `QJsonObject` when they have no useful typed contract.

Supported fields are `QString`, `bool`, integer and floating-point types, `JSON_ENUMERATION` wrappers, nested `JSON_STRUCT` types, `QList<T>` (including `QStringList`), and `QVariantMap`. Use `QVariantMap` for deliberately variable-shaped data, such as capability state. The `is_optional` trait exists, but conversion for `std::optional<T>` is not implemented. Types such as `QHostAddress` also need application-layer conversion from a supported JSON type.

## Parsing, validation, and absent fields

`From` maps an already parsed `QJsonObject`; it does not parse bytes or validate a schema. It ignores unknown keys and uses Qt conversions for present values of the wrong type. Check document shape, input types, allowed keys, and domain constraints where the input contract needs those checks. Use a floating-point JSON field and check integrality before narrowing when fractions must be rejected; decoding directly into an integer loses that evidence.

`From` constructs `T result;` and assigns only present fields. Missing `QString` and list fields retain their class defaults, but missing scalar members of a `JSON_STRUCT` may be uninitialized. Do not claim the engine provides scalar defaults or read an absent scalar. For required fields, validate presence before decoding. For optional configuration fields, one public-API approach is to serialize a fully initialized defaults object, overlay supplied keys, then decode the complete object:

```cpp
auto complete = Serialization::To(ServerFields{
  .host = "127.0.0.1", .port = 0, .oauth_token = {}});
for (auto it = object.begin(); it != object.end(); ++it) {
  // Reject unknown keys here if required by the input contract.
  complete.insert(it.key(), it.value());
}
const auto fields = Serialization::From<ServerFields>(complete);
```

Keep the original object when absent and explicit values have different meanings. For example, test `object.contains("port")` before assigning an application's optional port; the decode default is not a requested port. A present `null` must not silently become an omitted value when the contract rejects it.

`JsonLoader::Load<T>(path)` wraps file loading and `From`, returning `std::optional<T>`. Reuse it for trusted bundled files following `UnitsList`. Its raw loader reads the whole file, logs file failures, and does not check JSON parse errors or root shape. For bounded runtime files with specific errors or sensitive values, keep the required file/document checks and use `From` for typed decoding, following the document-to-DTO pattern in `YandexAccountApi`. Do not weaken those checks to fit the helper or log a token-bearing object.

## Shared-library codecs and verification

For API model codecs already exported across a shared-library boundary, preserve the `extern template` declarations and matching exported instantiations in [src/api/model/Serialization.cpp](../../../src/api/model/Serialization.cpp). Private DTOs use normal header instantiation; they do not need exported codecs.

Build affected targets and run deterministic tests for observable decoding/encoding behavior. The engine's tests run in the `All` CTest suite; REST configuration and worker coverage run in `Rest` and `RestProcess`. Use [docs/build.md](../../../docs/build.md) for build commands. When adding defaults or strict validation, cover omitted fields, explicit null/wrong types, and constraints that could be lost during conversion. Use synthetic token values and keep them out of diagnostics. Extend the engine only when the requested behavior needs a capability it lacks, and test that capability without rewriting unrelated callers.
