#include "ApiTests.h"

void ApiTests::TestCapabilities() {
  const QByteArray test_content = R"({
    "retrievable": true,
    "type": "devices.capabilities.on_off",
    "parameters": {
      "split": true
    },
    "state": {
      "instance": "on",
			"value": true
    },
    "last_updated": 123.45
  })";
  QJsonDocument test_json = QJsonDocument::fromJson(test_content);

  auto capability_object = Serialization::From<CapabilityObject>(test_json.object());

  auto state = capability_object.state;

  QCOMPARE(state["instance"], "on");
  QCOMPARE(state["value"], "true");

  auto parameters = capability_object.parameters;

  QCOMPARE(parameters["split"].toBool(), true);
}

void ApiTests::CapabilitiesNullTest() {
  const QByteArray test_content = R"({
    "retrievable": true,
    "type": "devices.capabilities.on_off",
    "parameters": {
      "split": false
    },
    "state": null,
    "last_updated": 123.45
  })";
  QJsonDocument test_json = QJsonDocument::fromJson(test_content);

  auto capability_object = Serialization::From<CapabilityObject>(test_json.object());

  auto state = capability_object.state;

  QVERIFY(state.isEmpty());
}

void ApiTests::CapabilitiesObjectTest() {
  const QByteArray test_content = R"({
    "retrievable": true,
    "type": "devices.capabilities.on_off",
    "parameters": {
      "split": true
    },
    "state": {
      "instance": "on",
			"value": {
        "h": 123,
        "s": 321,
        "v": 111
      }
    },
    "last_updated": 123.45
  })";
  QJsonDocument test_json = QJsonDocument::fromJson(test_content);

  auto capability_object = Serialization::From<CapabilityObject>(test_json.object());

  auto state = capability_object.state;

  QCOMPARE(state["instance"], "on");

  auto color = state["value"].toMap();

  QCOMPARE(color["h"].toInt(), 123);
  QCOMPARE(color["s"].toInt(), 321);
  QCOMPARE(color["v"].toInt(), 111);

  auto parameters = capability_object.parameters;

  QCOMPARE(parameters["split"].toBool(), true);
}

void ApiTests::PropertiesTest() {
  const QByteArray test_content = R"({
		"status": "ok",
		"request_id": "",
		"id": "",
		"name": "",
		"aliases": [""],
		"type": "devices.types.iron",
		"state": "",
		"groups": [""],
		"room": "",
		"external_id": "",
		"skill_id": "",
		"capabilities": [{
				"retrievable": false,
				"type": "devices.capabilities.on_off",
				"parameters": {},
				"state": {},
				"last_updated": 0.0
			},
			{
				"retrievable": false,
				"type": "devices.capabilities.range",
				"parameters": {},
				"state": {},
				"last_updated": 1.23
			}
		],
		"properties": [{
				"retrievable": false,
				"type": "devices.properties.float",
				"parameters": {},
				"state": {},
				"last_updated": 0
	        },
	        {
				"retrievable": false,
				"type": "devices.properties.event",
				"parameters": {},
				"state": {},
				"last_updated": 94
	        }
	    ]
	})";
  QJsonDocument test_json = QJsonDocument::fromJson(test_content);


	auto object = Serialization::From<DeviceObject>(test_json.object());

	QCOMPARE(object.properties.size(), 2);
}
