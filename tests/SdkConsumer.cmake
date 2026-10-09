function(run_checked)
  execute_process(COMMAND ${ARGV} RESULT_VARIABLE result
    OUTPUT_VARIABLE output ERROR_VARIABLE error)
  if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "SDK consumer failed (${ARGV}): ${result}\n${output}\n${error}")
  endif()
endfunction()

set(prefix "${BUILD_PATH}/sdk-test/${CONFIGURATION}/stage")
set(consumer "${BUILD_PATH}/sdk-test/${CONFIGURATION}/consumer")
run_checked("${CMAKE_COMMAND}" --install "${BUILD_PATH}" --config "${CONFIGURATION}"
  --prefix "${prefix}" --component SDK)
run_checked("${CMAKE_COMMAND}" --fresh -S "${SOURCE_PATH}/examples/sdk" -B "${consumer}"
  -G "${GENERATOR}" "-DCMAKE_CXX_COMPILER=${COMPILER}"
  "-DCMAKE_BUILD_TYPE=${CONFIGURATION}" "-DCMAKE_PREFIX_PATH=${prefix}"
  "-DQt6_DIR=${QT_PACKAGE_PATH}")
run_checked("${CMAKE_COMMAND}" --build "${consumer}" --config "${CONFIGURATION}" --parallel 2)
if(WIN32)
  # Installed SDK DLLs must be available to the independent executable.
  set(ENV{PATH} "${prefix}/bin;$ENV{PATH}")
endif()
run_checked("${CMAKE_CTEST_COMMAND}" --test-dir "${consumer}" -C "${CONFIGURATION}" --output-on-failure)
