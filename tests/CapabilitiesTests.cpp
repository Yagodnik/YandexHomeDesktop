#include "CapabilitiesTests.h"

#include <QColor>
#include "iot/capabilities/ColorSettingCapability.h"
#include "iot/capabilities/OnOffCapability.h"
#include "iot/capabilities/RangeCapability.h"
#include "iot/capabilities/ToggleCapability.h"

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
