#include <QSignalSpy>
#include <QTest>
#include <memory>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtQuickTest/quicktest.h>
#include "QmlTestEnvironment.h"
#include <algorithm>
#include "../support/HomeTestFixtures.h"

using namespace home_test;

class HomeUiTests final : public QObject {
  Q_OBJECT
private slots:
  void initTestCase() { QQuickStyle::setStyle("Basic"); }

  void ExpandingLargeSectionKeepsItsHeadingInPlace_data() {
    QTest::addColumn<int>("leading_room_count");
    QTest::addColumn<bool>("favorites");
    QTest::addColumn<int>("viewport_height");
    QTest::newRow("first-room") << 0 << false << 230;
    QTest::newRow("room-after-scrolling") << 12 << false << 230;
    QTest::newRow("room-in-taller-window") << 12 << false << 460;
    QTest::newRow("favorites") << 0 << true << 230;
  }

  void ExpandingLargeSectionKeepsItsHeadingInPlace() {
    QFETCH(int, leading_room_count);
    QFETCH(bool, favorites);
    QFETCH(int, viewport_height);
    HomeTestApi api; HomeService service(&api);
    HomeTestAccountApi account_api; AccountModel account(&account_api);
    Settings settings(nullptr, true);
    QStringList collapsed_rooms{"room-one", "room-tail"};
    for (int i = 0; i < leading_room_count; ++i) { collapsed_rooms.append("leading-" + QString::number(i)); }
    settings.SetCollapsedRooms("account-one", collapsed_rooms);
    settings.SetFavoritesCollapsed("account-one", true);
    HomeViewModel view_model(&service, &settings, &account);
    QQmlEngine engine;
    initializeQmlTestEnvironment(&engine);
    engine.addImportPath(HOME_IMPORT_PATH);
    QSignalSpy warnings(&engine, &QQmlEngine::warnings);
    QQmlComponent component(&engine);
    component.loadFromModule("YandexHomeDesktop.Pages", "DevicesPage");
    QTRY_VERIFY(component.status() != QQmlComponent::Loading);
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    std::unique_ptr<QObject> page(component.createWithInitialProperties({{"viewModel", QVariant::fromValue(&view_model)}}));
    QVERIFY2(page, qPrintable(component.errorString()));
    auto* item = qobject_cast<QQuickItem*>(page.get()); QVERIFY(item);
    QQuickWindow window; window.resize(350, viewport_height);
    item->setParentItem(window.contentItem()); item->setWidth(350); item->setHeight(viewport_height); window.show();
    auto snapshot = Snapshot();
    for (int i = 0; i < leading_room_count; ++i) {
      RoomObject room{}; room.id = "leading-" + QString::number(i);
      room.name = "A room " + QString::number(i); room.household_id = "one";
      snapshot.rooms.append(room);
    }
    for (int i = 0; i < 20; ++i) {
      auto device = snapshot.devices.first(); device.id = "lamp-" + QString::number(i);
      snapshot.devices.append(device);
    }
    RoomObject tail{}; tail.id = "room-tail"; tail.name = "Z other room"; tail.household_id = "one";
    snapshot.rooms.append(tail);
    if (favorites) {
      QStringList favorite_ids;
      for (const auto& device : snapshot.devices) {
        if (device.household_id == "one") { favorite_ids.append(device.id); }
      }
      settings.SetFavoriteDevices("account-one", favorite_ids);
    }
    account_api.requests.last().Reply(AccountInfo{"User", {}, {}, "account-one"});
    api.requests.last().Reply(snapshot);
    QQuickItem* list = nullptr;
    QTRY_VERIFY((list = FindItem(item, "roomsList")) != nullptr);
    auto* favorites_body = FindItem(item, "sectionBody-favorites"); QVERIFY(favorites_body);
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    QTRY_COMPARE(favorites_body->height(), 0.0);
    QVERIFY(list->setProperty("contentY", std::max(0.0, list->property("contentHeight").toDouble() - list->height())));
    const QString section_id = favorites ? "favorites" : "room-one";
    QQuickItem* heading = nullptr;
    QTRY_VERIFY((heading = FindItem(item, "collapse-" + section_id)) != nullptr);
    auto* body = FindItem(item, "sectionBody-" + section_id); QVERIFY(body);
    QTRY_COMPARE(body->height(), 0.0);
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    const auto initial_y = heading->mapToItem(list, QPointF()).y();
    const auto initial_scroll = list->property("contentY").toDouble();
    QVERIFY(initial_y >= 0 && initial_y + heading->height() <= list->height());
    qreal maximum_shift = 0;
    const auto track_heading = [&] {
      maximum_shift = std::max(maximum_shift, qAbs(heading->mapToItem(list, QPointF()).y() - initial_y));
    };
    const auto tracking = connect(&window, &QQuickWindow::afterAnimating, &window, track_heading);
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
      heading->mapToScene(QPointF(heading->width() / 2, heading->height() / 2)).toPoint());
    QTRY_COMPARE(body->height(), body->property("expandedHeight").toDouble());
    // The enclosing Column updates its scroll extent during polish, after the
    // height animation reaches its final value.
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    QVERIFY(body->height() > list->height());
    QVERIFY2(maximum_shift < 1, qPrintable(QString("Heading moved by %1 pixels during expansion").arg(maximum_shift)));
    QVERIFY(qAbs(heading->mapToItem(list, QPointF()).y() - initial_y) < 1);
    QCOMPARE(list->property("contentY").toDouble(), initial_scroll);
    disconnect(tracking);

    // All rows remain reachable in the same viewport after the animation finishes.
    QVERIFY(list->setProperty("contentY", list->property("contentHeight").toDouble() - list->height()));
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    auto* last_device = FindItem(body, "device-lamp-19"); QVERIFY(last_device);
    const auto last_y = last_device->mapToItem(list, QPointF()).y();
    QVERIFY2(last_y >= 0 && last_y + last_device->height() <= list->height(),
      qPrintable(QString("Last device y=%1, height=%2, viewport=%3, contentY=%4, contentHeight=%5")
        .arg(last_y).arg(last_device->height()).arg(list->height())
        .arg(list->property("contentY").toDouble()).arg(list->property("contentHeight").toDouble())));
    QVERIFY(list->setProperty("contentY", initial_scroll));
    maximum_shift = 0;
    const auto collapse_tracking = connect(&window, &QQuickWindow::afterAnimating, &window, track_heading);
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
      heading->mapToScene(QPointF(heading->width() / 2, heading->height() / 2)).toPoint());
    QTRY_COMPARE(body->height(), 0.0);
    QVERIFY2(maximum_shift < 1, qPrintable(QString("Heading moved by %1 pixels during collapse").arg(maximum_shift)));
    QVERIFY(qAbs(heading->mapToItem(list, QPointF()).y() - initial_y) < 1);
    disconnect(collapse_tracking);
    QCOMPARE(warnings.size(), 0);
  }

  void RoomCollapseAndFavoriteClicksReachViewModel() {
    HomeTestApi api; HomeService service(&api);
    HomeTestAccountApi account_api; AccountModel account(&account_api);
    Settings settings(nullptr, true);
    HomeViewModel view_model(&service, &settings, &account);
    QQmlEngine engine;
    initializeQmlTestEnvironment(&engine);
    engine.addImportPath(HOME_IMPORT_PATH);
    QSignalSpy warnings(&engine, &QQmlEngine::warnings);
    QQmlComponent component(&engine);
    component.loadFromModule("YandexHomeDesktop.Pages", "DevicesPage");
    QTRY_VERIFY(component.status() != QQmlComponent::Loading);
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    std::unique_ptr<QObject> page(component.createWithInitialProperties({{"viewModel", QVariant::fromValue(&view_model)}}));
    QVERIFY2(page, qPrintable(component.errorString()));
    auto* item = qobject_cast<QQuickItem*>(page.get()); QVERIFY(item);
    QQuickWindow window; window.resize(350, 460);
    item->setParentItem(window.contentItem()); item->setWidth(350); item->setHeight(460); window.show();
    auto snapshot = Snapshot();
    auto extra = snapshot.devices.first(); extra.id = "extra"; extra.name = QString(200, 'X');
    snapshot.devices.append(extra);
    api.requests.last().Reply(snapshot);
    account_api.requests.last().Reply(AccountInfo{"User", {}, {}, "account-one"});
    QQuickItem* collapse = nullptr;
    QTRY_VERIFY((collapse = FindItem(item, "collapse-room-one")) != nullptr);
    auto* room_devices = FindItem(item, "roomDevices-room-one"); QVERIFY(room_devices);
    auto* room_body = FindItem(item, "sectionBody-room-one"); QVERIFY(room_body);
    auto* arrow = FindItem(collapse, "sectionArrow"); QVERIFY(arrow);
    auto* title = FindItem(collapse, "sectionTitle"); QVERIFY(title);
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    QTRY_VERIFY(room_devices->height() > 0);
    QTRY_COMPARE(room_body->height(), room_body->property("expandedHeight").toDouble());
    QCOMPARE(arrow->x() - title->x() - title->width(), 8.0);
    QVERIFY(arrow->property("background").isNull());
    auto click = [&window](QQuickItem* target) {
      QVERIFY(QQuickTest::qWaitForPolish(&window));
      QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
        target->mapToScene(QPointF(target->width() / 2, target->height() / 2)).toPoint());
    };
    click(collapse);
    QTRY_VERIFY(room_body->height() > 0 && room_body->height() < room_body->property("expandedHeight").toDouble());
    QTRY_VERIFY(arrow->rotation() > 90 && arrow->rotation() < 180);
    QTRY_VERIFY(!room_devices->isVisible());
    QCOMPARE(room_body->height(), 0.0);
    QTRY_COMPARE(arrow->rotation(), 90.0);
    QCOMPARE(settings.GetCollapsedRooms("account-one"), QStringList{"room-one"});
    click(collapse);
    QTRY_VERIFY(room_body->height() > 0 && room_body->height() < room_body->property("expandedHeight").toDouble());
    QTRY_COMPARE(room_body->height(), room_body->property("expandedHeight").toDouble());
    QTRY_COMPARE(arrow->rotation(), 180.0);
    QTRY_VERIFY(room_devices->isVisible());
    QVERIFY(settings.GetCollapsedRooms("account-one").isEmpty());
    auto* first_star = FindItem(room_devices, "favorite-extra"); QVERIFY(first_star);
    click(first_star);
    QCOMPARE(view_model.GetFavorites()->rowCount(), 1);
    QQuickItem* favorites_section = nullptr;
    QTRY_VERIFY((favorites_section = FindItem(item, "favoritesSection")) && favorites_section->isVisible());
    QQuickItem* favorite_star = nullptr;
    QTRY_VERIFY((favorite_star = FindItem(favorites_section, "favorite-extra")) != nullptr);
    auto* rooms_list = FindItem(item, "roomsList"); QVERIFY(rooms_list);
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    QVERIFY(rooms_list->setProperty("contentY", 0.0));
    QTRY_VERIFY(favorite_star->mapToItem(rooms_list, QPointF()).y() >= 0);
    auto* favorites_collapse = FindItem(favorites_section, "collapse-favorites"); QVERIFY(favorites_collapse);
    auto* favorites_body = FindItem(favorites_section, "sectionBody-favorites"); QVERIFY(favorites_body);
    QTRY_COMPARE(favorites_body->height(), favorites_body->property("expandedHeight").toDouble());
    click(favorites_collapse);
    QVERIFY(view_model.AreFavoritesCollapsed());
    QVERIFY(settings.GetFavoritesCollapsed("account-one"));
    QTRY_VERIFY(favorites_body->height() > 0 && favorites_body->height() < favorites_body->property("expandedHeight").toDouble());
    QTRY_COMPARE(favorites_body->height(), 0.0);
    click(favorites_collapse);
    QVERIFY(!view_model.AreFavoritesCollapsed());
    QTRY_COMPARE(favorites_body->height(), favorites_body->property("expandedHeight").toDouble());
    auto* router = qobject_cast<QmlTestModel*>(engine.rootContext()->contextProperty("router").value<QObject*>());
    auto* device_api = qobject_cast<QmlTestDeviceApi*>(engine.rootContext()->contextProperty("deviceTestApi").value<QObject*>());
    QVERIFY(router); QVERIFY(device_api);
    QCOMPARE(router->callCount, 0); // The star does not navigate to device controls.
    QVERIFY(QQuickTest::qWaitForPolish(&window));
    auto* favorite_row = FindItem(favorites_section, "device-extra"); QVERIFY(favorite_row);
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
      favorite_row->mapToScene(QPointF(100, favorite_row->height() / 2)).toPoint());
    QCOMPARE(router->lastRoute, QString("device"));
    QCOMPARE(device_api->property("deviceId").toString(), QString("extra"));
    click(favorite_star);
    QCOMPARE(view_model.GetFavorites()->rowCount(), 0);
    QVERIFY(favorites_section->isVisible());
    auto* empty_message = FindItem(favorites_section, "sectionEmpty-favorites"); QVERIFY(empty_message);
    QTRY_VERIFY(empty_message->isVisible());
    QVERIFY(settings.GetFavoriteDevices("account-one").isEmpty());
    QCOMPARE(warnings.size(), 0);
  }

  void PageStateAndHouseholdClicksReachViewModel() {
    HomeTestApi api; HomeService service(&api); HomeViewModel view_model(&service);
    QQmlEngine engine;
    initializeQmlTestEnvironment(&engine);
    engine.addImportPath(HOME_IMPORT_PATH);
    engine.rootContext()->setContextProperty("homeViewModel", &view_model);
    QSignalSpy warnings(&engine, &QQmlEngine::warnings);
    QQmlComponent component(&engine);
    component.loadFromModule("YandexHomeDesktop.Pages", "MainPage");
    QTRY_VERIFY(component.status() != QQmlComponent::Loading);
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    std::unique_ptr<QObject> page(component.create());
    QVERIFY2(page, qPrintable(component.errorString()));
    auto* item = qobject_cast<QQuickItem*>(page.get()); QVERIFY(item);
    QQuickWindow window; window.resize(350, 460);
    item->setParentItem(window.contentItem()); item->setWidth(350); item->setHeight(460); window.show();
    auto* states = page->findChild<QObject*>("homeStates");
    auto* refresh = page->findChild<QQuickItem*>("homeRefreshButton");
    auto* picker = page->findChild<QQuickItem*>("householdPicker");
    QVERIFY(states); QVERIFY(refresh); QVERIFY(picker);
    QCOMPARE(api.requests.size(), 1);
    QCOMPARE(states->property("currentIndex").toInt(), 0);
    QVERIFY(!refresh->isEnabled());
    api.requests.last().Reply(std::unexpected(ApiError{ApiErrorKind::Network, "offline"}));
    QCOMPARE(states->property("currentIndex").toInt(), 1);
    QVERIFY(refresh->isEnabled());
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
      refresh->mapToScene(QPointF(refresh->width() - 20, refresh->height() / 2)).toPoint());
    QCOMPARE(api.requests.size(), 2);
    api.requests.last().Reply(Snapshot());
    QCOMPARE(states->property("currentIndex").toInt(), 2);
    QVERIFY(!picker->property("loading").toBool());
    QVERIFY(QMetaObject::invokeMethod(picker, "open"));
    QQuickItem* row = nullptr;
    QTRY_VERIFY((row = FindItem(picker, "household-two")) != nullptr);
    QTest::qWait(350); // Allow the sheet's existing opening animation to complete.
    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
      row->mapToScene(QPointF(row->width() / 2, row->height() / 2)).toPoint());
    QCOMPARE(view_model.GetCurrentHousehold(), QString("two"));
    QVERIFY(!picker->property("opened").toBool());
    QCOMPARE(warnings.size(), 0);
  }
private:
  static QQuickItem* FindItem(QQuickItem* item, const QString& name) {
    if (item->objectName() == name) { return item; }
    for (auto* child : item->childItems()) {
      if (auto* result = FindItem(child, name)) { return result; }
    }
    return nullptr;
  }
};

QTEST_MAIN(HomeUiTests)
#include "HomeUiTests.moc"
