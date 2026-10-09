# A GUI launch must never become a CLI command or REST daemon.
foreach(argument --enable-rest --serve-rest --list-devices)
  execute_process(COMMAND "${APP_PATH}" "${argument}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 5)
  if(NOT result EQUAL 2 OR NOT output STREQUAL "" OR error STREQUAL "")
    message(FATAL_ERROR "GUI accepted a headless command: ${argument}: ${result} ${output}${error}")
  endif()
endforeach()
