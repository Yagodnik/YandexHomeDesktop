#pragma once

#include <QObject>

class ModelBoundaryTests final : public QObject {
  Q_OBJECT
private slots:
  void HomeRefreshUpdatesAllModels();
  void ScenarioResultsRemainScoped();
  void DeviceDataFlowsThroughController();
  void ImmediateDeviceResultSurvivesReset();
  void ActionEventsReachController();
  void AccountModelKeepsQmlContract();
};
