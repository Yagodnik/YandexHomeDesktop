#include "CapabilitiesTests.h"

#include <QColor>
#include "iot/capabilities/ColorSettingCapability.h"
#include "iot/capabilities/OnOffCapability.h"
#include "iot/capabilities/RangeCapability.h"
#include "iot/capabilities/ToggleCapability.h"
#include "iot/capabilities/ModesCapability.h"
#include "iot/core/CapabilityState.h"
#include "iot/core/CapabilityRules.h"

void CapabilitiesTests::TestOnOff() {
  OnOffCapability capability;

  const auto on_state = capability.Create(true);

  QCOMPARE(on_state["instance"], "on");
  QCOMPARE(on_state["value"].toBool(), true);

  const auto off_state = capability.Create(false);

  QCOMPARE(off_state["instance"], "on");
  QCOMPARE(off_state["value"].toBool(), false);

  QCOMPARE(capability.GetValue().toBool(), false);

  capability.SetValue(true);

  QCOMPARE(capability.GetValue().toBool(), true);
}

void CapabilitiesTests::TestColorSetting() {
  ColorSettingCapability capability;

  /* Wrong color model */
  {
    QVariantMap params;

    params["color_model"] = "error!";

    capability.SetParameters(params);

    const auto new_state = capability.Create(QColor::fromRgb(1, 2, 3));

    QVERIFY(new_state.isEmpty());
  }
}

void CapabilitiesTests::TestRange() {
  RangeCapability capability;

  /* Test units */
  {
    QCOMPARE(capability.GetUnit(), "?");
  }

  /* Test null params */
  {
    Q_UNUSED(capability.Create(42.42));

    Q_UNUSED(capability.GetMin());
    Q_UNUSED(capability.GetMax());
    Q_UNUSED(capability.GetPrecision());

    capability.SetMin(123.52);
    capability.SetMax(425.52);
    capability.SetPrecision(2);

    QCOMPARE(capability.GetMin(), 123.52);
    QCOMPARE(capability.GetMax(), 425.52);
    QCOMPARE(capability.GetPrecision(), 2);
  }

  /* Test create */
  {
    const auto state = capability.Create(123.2);

    QCOMPARE(state["value"].toDouble(), 123.2);
  }
}

void CapabilitiesTests::TestToggle() {
  ToggleCapability capability;

  /* Test create */
  {
    QVariantMap params;
    params["instance"] = "pause";

    capability.SetParameters(params);

    const auto state = capability.Create(true);

    QCOMPARE(state["instance"], params["instance"]);
    QCOMPARE(state["value"], true);
  }
}

void CapabilitiesTests::SharedBuildersPreserveDesktopPayloads() {
  ColorSettingCapability color;
  color.SetParameters({{"color_model", "rgb"}});
  const auto rgb = color.Create(QColor::fromRgb(0x33, 0xaa, 0xee));
  QCOMPARE(rgb, (QVariantMap{{"instance", "rgb"}, {"value", quint32(0x33aaee)}}));
  QCOMPARE(rgb, Iot::State::Rgb(quint32(0x33aaee)));

  color.SetParameters({{"color_model", "hsv"}});
  const auto hsv = color.Create(QColor::fromHsv(120, 128, 200));
  QCOMPARE(hsv, (QVariantMap{{"instance", "hsv"}, {"value", QVariantMap{{"h", 120}, {"s", 50}, {"v", 78}}}}));
  QCOMPARE(hsv, Iot::State::Hsv(Iot::State::HsvComponents(120, 50, 78)));
  QVERIFY(Iot::ValidateInput(*Iot::Rules::Color(), hsv));

  // Preserve the existing QColor behavior for gray rather than silently fixing it.
  const auto gray = color.Create(QColor::fromRgb(128, 128, 128));
  QCOMPARE(gray.value("value").toMap().value("h").toInt(), -1);
  QVERIFY(!Iot::ValidateInput(*Iot::Rules::Color(), gray));
  QCOMPARE(color.Create(3000), Iot::State::Temperature(3000));
  QCOMPARE(color.Create(QString("sunrise")), Iot::State::Scene("sunrise"));

  RangeCapability range;
  range.SetParameters({{"instance", "volume"}});
  QCOMPARE(range.CreateRelative(-5), (QVariantMap{{"instance", "volume"}, {"value", -5.0}, {"relative", true}}));
  QCOMPARE(range.Create(50.5), Iot::State::Range("volume", 50.5));
  ModesCapability mode;
  mode.SetParameters({{"instance", "work_speed"}});
  QCOMPARE(mode.Create("auto"), (QVariantMap{{"instance", "work_speed"}, {"value", "auto"}}));
}

void CapabilitiesTests::SharedParametersPreserveDesktopDefaults() {
  RangeCapability range;
  QCOMPARE(range.GetMin(), 0.0); QCOMPARE(range.GetMax(), 0.0);
  QVERIFY(!range.GetRandomAccessSupport());
  range.SetParameters({{"random_access", true}, {"range", QVariantMap{{"min", -10.5}, {"max", 100.0}, {"precision", 0.5}}}});
  QCOMPARE(range.GetMin(), -10.5); QCOMPARE(range.GetMax(), 100.0);
  QCOMPARE(range.GetPrecision(), 0.5); QVERIFY(range.GetRandomAccessSupport());

  ColorSettingCapability color;
  QCOMPARE(color.GetTemperatureMin(), 0); QCOMPARE(color.GetTemperatureMax(), 0);
  QVERIFY(!color.GetSupportsColors()); QVERIFY(!color.GetSupportsTemperature());
  const QVariantList scenes{QVariantMap{{"id", "sunrise"}}};
  color.SetParameters({{"color_model", "rgb"}, {"temperature_k", QVariantMap{{"min", 2000}, {"max", 6500}}},
    {"color_scene", QVariantMap{{"scenes", scenes}}}});
  QCOMPARE(color.GetTemperatureMin(), 2000); QCOMPARE(color.GetTemperatureMax(), 6500);
  QCOMPARE(color.GetAvailableScenes(), scenes);
  QVERIFY(color.GetSupportsColors()); QVERIFY(color.GetSupportsTemperature());
}
