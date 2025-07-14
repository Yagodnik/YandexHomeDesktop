#pragma once
#include "IExecutor.h"

class OnOffExecutor final : public IExecutor {
public:
  void Execute(const QString& value) override;
  void PrintInfo() override;
};
