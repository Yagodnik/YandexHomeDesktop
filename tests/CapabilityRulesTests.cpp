#include <QTest>
#include <limits>
#include "iot/core/CapabilityRules.h"
#include "iot/core/CapabilityState.h"

// Qt Core/Test only: CLI, QML, or QColor aren't needed to use domain rules.
class CapabilityRulesTests final : public QObject {
  Q_OBJECT
private slots:
  void ColorChecksTypedValuesWithoutCoercingStringsOrBooleans() {
    const auto color = Iot::Rules::Color();
    for (const auto& component : {QVariant("120"), QVariant(true), QVariant(std::numeric_limits<double>::infinity())}) {
      const auto hsv = QVariantMap{{"h", component}, {"s", 50}, {"v", 50}};
      QVERIFY(!Iot::ValidateInput(*color, Iot::State::Hsv(hsv)));
    }
    QVERIFY(Iot::ValidateInput(*color, Iot::State::Hsv({{"h", 360}, {"s", 100}, {"v", 0}})));
    QVERIFY(Iot::ValidateInput(*color, Iot::State::Rgb(quint32(0xffffff))));
    QVERIFY(!Iot::ValidateInput(*color, Iot::State::Rgb(0x1000000)));
    QVERIFY(!Iot::ValidateInput(*color, Iot::State::Temperature(3000.5)));
    QVERIFY(!Iot::ValidateInput(*color, Iot::State::Color("unknown", 10)));
  }
  void ColorMetadataSupportsSeveralInstancesInOneCapability() {
    const auto rules = Iot::Rules::Color();
    CapabilityObject capability{};
    capability.type = CapabilityType::ColorSetting;
    capability.parameters = {{"color_model", "rgb"}, {"temperature_k", QVariantMap{{"min", 2000}, {"max", 6500}}},
      {"color_scene", QVariantMap{{"scenes", QVariantList{QVariantMap{{"id", "sunrise"}}}}}}};
    QVERIFY(rules->Matches(capability, Iot::State::Rgb(0x33aaee)));
    QVERIFY(rules->Matches(capability, Iot::State::Temperature(3000)));
    QVERIFY(rules->Matches(capability, Iot::State::Scene("sunrise")));
    QVERIFY(!rules->Matches(capability, Iot::State::Hsv({{"h", 120}, {"s", 50}, {"v", 50}})));
    QVERIFY(rules->ValidateDevice(capability.parameters, Iot::State::Temperature(2000)));
    QVERIFY(rules->ValidateDevice(capability.parameters, Iot::State::Temperature(6500)));
    QVERIFY(!rules->ValidateDevice(capability.parameters, Iot::State::Temperature(6501)));
    QVERIFY(rules->ValidateDevice(capability.parameters, Iot::State::Scene("sunrise")));
    QVERIFY(!rules->ValidateDevice(capability.parameters, Iot::State::Scene("unknown")));
  }
  void RangeRulesKeepRelativeChangesSeparateFromAbsoluteLimits() {
    const auto rules = Iot::Rules::Range();
    const QVariantMap parameters{{"random_access", false}, {"range", QVariantMap{{"min", 0}, {"max", 100}}}};
    const auto absolute = Iot::State::Range("volume", 50);
    const auto relative = Iot::State::RelativeRange("volume", -5);
    QVERIFY(Iot::ValidateInput(*rules, absolute)); QVERIFY(Iot::ValidateInput(*rules, relative));
    QVERIFY(!rules->ValidateDevice(parameters, absolute)); QVERIFY(rules->ValidateDevice(parameters, relative));
    // Missing metadata remains permissive, as in the previous CLI contract.
    QVERIFY(rules->ValidateDevice({}, absolute));
    QVERIFY(!Iot::ValidateInput(*rules, Iot::State::Range("volume", std::numeric_limits<double>::quiet_NaN())));
    QVERIFY(!Iot::ValidateInput(*Iot::Rules::Toggle(), Iot::State::WithRelative(Iot::State::Toggle("mute", true))));
  }
  void InstanceMatchingAndModeValidationUseDeviceMetadata() {
    CapabilityObject capability{};
    capability.type = CapabilityType::Range;
    capability.parameters = {{"instance", "brightness"}};
    capability.state = {{"instance", "volume"}};
    const auto rules = Iot::Rules::Range();
    QVERIFY(rules->Matches(capability, Iot::State::Range("brightness", 50)));
    QVERIFY(!rules->Matches(capability, Iot::State::Range("volume", 50)));
    capability.type = CapabilityType::OnOff;
    QVERIFY(!rules->Matches(capability, Iot::State::Range("brightness", 50)));
    capability.type = CapabilityType::Range;
    capability.parameters.clear();
    QVERIFY(rules->Matches(capability, Iot::State::Range("volume", 50)));
    const auto mode = Iot::Rules::Mode();
    const QVariantMap parameters{{"modes", QVariantList{QVariantMap{{"value", "auto"}}}}};
    QVERIFY(mode->ValidateDevice(parameters, Iot::State::Mode("work_speed", "auto")));
    QVERIFY(!mode->ValidateDevice(parameters, Iot::State::Mode("work_speed", "turbo")));
  }
};
QTEST_GUILESS_MAIN(CapabilityRulesTests)
#include "CapabilityRulesTests.moc"
