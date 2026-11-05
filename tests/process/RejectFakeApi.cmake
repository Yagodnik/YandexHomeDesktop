execute_process(COMMAND "${APP_PATH}" --fake-api
  RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT result EQUAL 2 OR NOT "${output}${error}" MATCHES "only in Debug builds")
  message(FATAL_ERROR "Release must reject --fake-api before startup: ${result}\n${output}${error}")
endif()
