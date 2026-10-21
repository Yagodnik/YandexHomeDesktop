#pragma once

#include "yh/appcli_export.h"
#include "cli/CommandRegistry.h"

class ScenarioCommands {
  Q_DECLARE_TR_FUNCTIONS(ScenarioCommands)
};
APPCLI_EXPORT void RegisterScenarioCommands(CommandRegistry& registry);
