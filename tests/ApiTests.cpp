#include "ApiTests.h"

#include <QJsonDocument>

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

// These tests deliberately include only model headers: the codec bodies must
// come from YandexHomeApi, while custom types still use Serialization.h.
void ApiTests::SharedResponseCodecs() {
  const auto object = QJsonDocument::fromJson(R"({
    "status": "ok", "message": "ready", "request_id": "request-1",
    "rooms": [{"id": "room-1", "name": "Kitchen", "devices": ["device-1"]}],
    "groups": [{"id": "group-1", "capabilities": []}],
    "devices": [{
      "id": "device-1", "name": "Lamp", "aliases": ["light"],
      "capabilities": [{"type": "devices.capabilities.on_off", "retrievable": true,
        "state": {"instance": "on", "value": true}, "last_updated": 123.5}],
      "properties": [{"type": "devices.properties.float", "retrievable": true,
        "state": {"instance": "humidity", "value": 42.5}, "last_updated": 123.5}]
    }],
    "scenarios": [{"id": "scenario-1", "name": "Evening", "is_active": true}],
    "households": [{"id": "household-1", "name": "Home"}]
  })").object();

  const auto user = Serialization::From<UserInfo>(object);
  QCOMPARE(user.status, Status::Ok);
  QCOMPARE(user.rooms.size(), 1);
  QCOMPARE(user.rooms.front().devices, QStringList{"device-1"});
  QCOMPARE(user.groups.front().id, "group-1");
  QCOMPARE(user.devices.front().aliases, QStringList{"light"});
  QCOMPARE(user.devices.front().capabilities.front().type, CapabilityType::OnOff);
  QCOMPARE(user.devices.front().properties.front().type, PropertyType::Float);
  QCOMPARE(user.devices.front().properties.front().state.value("value").toDouble(), 42.5);
  QCOMPARE(user.scenarios.front().is_active, true);
  QCOMPARE(user.households.front().name, "Home");

  auto device_object = object.value("devices").toArray().at(0).toObject();
  device_object["status"] = "ok";
  device_object["state"] = "online";
  const auto device = Serialization::From<DeviceInfo>(device_object);
  QCOMPARE(device.status, Status::Ok);
  QCOMPARE(device.state, DeviceState::Online);
  QCOMPARE(device.capabilities.size(), 1);
  QCOMPARE(device.properties.size(), 1);

  const auto response = Serialization::From<Response>(object);
  QCOMPARE(response.request_id, "request-1");
  QCOMPARE(response.message, "ready");
  QCOMPARE(response.status, Status::Ok);

  const auto actions = Serialization::From<DeviceActionResponse>(QJsonDocument::fromJson(R"({
    "status": "ok", "request_id": "request-2",
    "devices": [{"id": "device-1", "capabilities": [{
      "type": "devices.capabilities.on_off", "state": {
        "instance": "on", "action_result": {"status": "DONE", "error_code": ""}
      }
    }]}]
  })").object());
  QCOMPARE(actions.status, Status::Ok);
  QCOMPARE(actions.devices.front().id, "device-1");
  QCOMPARE(actions.devices.front().capabilities.front().state.instance, "on");
  QCOMPARE(actions.devices.front().capabilities.front().state.action_result.status, "DONE");
}

void ApiTests::SharedActionCodec() {
  const CapabilityObject capability{
    .type = CapabilityType::OnOff,
    .retrievable = true,
    .state = {{"instance", "on"}, {"value", true}},
    .parameters = {{"split", true}},
    .last_updated = 123.5
  };
  const DeviceActionsObject action{.id = "device-1", .actions = {capability}};

  const auto object = Serialization::To(action);
  QCOMPARE(object.value("id").toString(), "device-1");
  const auto actions = object.value("actions").toArray();
  QCOMPARE(actions.size(), 1);
  const auto encoded = actions.at(0).toObject();
  QCOMPARE(encoded.value("type").toString(), "devices.capabilities.on_off");
  QCOMPARE(encoded.value("state").toObject().value("value").toBool(), true);

  const auto decoded = Serialization::From<CapabilityObject>(encoded);
  QCOMPARE(decoded.type, capability.type);
  QCOMPARE(decoded.state, capability.state);
  QCOMPARE(decoded.parameters, capability.parameters);
  QCOMPARE(decoded.last_updated, capability.last_updated);
}
