add_library(AppCli STATIC
  ICommand.h CliContext.h CliArguments.h CliArguments.cpp
  CommandRegistry.h CommandRegistry.cpp CliCommand.h CliCommand.cpp
  CapabilityCommands.h CapabilityCommands.cpp
  capabilities/Builtins.h capabilities/Input.h
  capabilities/ValueParsers.h capabilities/ValueParsers.cpp
  capabilities/BooleanInput.cpp capabilities/RangeInput.cpp
  capabilities/ModeInput.cpp capabilities/ColorInput.cpp CliOutput.h CliOutput.cpp
  CliRunner.h CliRunner.cpp
  commands/DeviceCommands.h commands/DeviceCommands.cpp
  commands/ScenarioCommands.h commands/ScenarioCommands.cpp
  commands/LocalCommands.h commands/LocalCommands.cpp)
target_link_libraries(AppCli PUBLIC AppServices IotCore Qt6::Core)
