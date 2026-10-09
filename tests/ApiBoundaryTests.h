#pragma once

#include <QObject>

class ApiBoundaryTests final : public QObject {
  Q_OBJECT
private slots:
  void HomeRequestsAreScoped();
  void ParsingAndScenarioResults();
  void ActionResultIsAggregated();
  void AccountResults();
  void TimeoutReportsOnce();
  void DestroyedContextSuppressesEvents();
  void MissingCredentialsSkipTransport();
};
