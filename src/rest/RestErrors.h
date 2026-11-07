#pragma once

#include "yh/apprest_export.h"

#include "RestReply.h"
#include "services/CommandResult.h"

APPREST_EXPORT void FailApi(RestReply* reply, const ApiError& error);
APPREST_EXPORT void FailCommand(RestReply* reply, const CommandError& error);
