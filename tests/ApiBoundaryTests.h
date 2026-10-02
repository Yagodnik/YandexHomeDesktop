#pragma once

#include <QObject>

class ApiBoundaryTests final : public QObject {
  Q_OBJECT
private slots:
  void HomeRequestsAreScoped();
  void ParsingAndScenarioResults();
  void ActionEventsArePreserved();
  void AccountResults();
  void TimeoutReportsBothLegacyErrors();
  void DestroyedContextSuppressesEvents();
};
