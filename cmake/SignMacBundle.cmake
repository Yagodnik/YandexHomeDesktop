set(_app "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/YandexHomeDesktop.app")
if(NOT IS_DIRECTORY "${_app}")
  message(FATAL_ERROR "Missing deployed app: ${_app}")
endif()

function(yh_sign_mac_code path)
  execute_process(COMMAND /usr/bin/codesign --force --sign - --timestamp=none "${path}"
    RESULT_VARIABLE _result OUTPUT_VARIABLE _output ERROR_VARIABLE _error)
  if(NOT "${_result}" STREQUAL "0")
    message(FATAL_ERROR "Signing ${path} failed: ${_result}\n${_output}${_error}")
  endif()
endfunction()

# Identify Mach-O files by their headers: framework binaries and helpers do not
# have file extensions. Skip symlinks so versioned libraries are signed once.
file(GLOB_RECURSE _files LIST_DIRECTORIES FALSE "${_app}/Contents/*")
foreach(_file IN LISTS _files)
  if(IS_SYMLINK "${_file}" OR _file STREQUAL "${_app}/Contents/MacOS/YandexHomeDesktop")
    continue()
  endif()
  file(READ "${_file}" _magic LIMIT 4 HEX)
  if(_magic MATCHES "^(feedface|cefaedfe|feedfacf|cffaedfe|cafebabe|bebafeca|cafebabf|bfbafeca)$")
    yh_sign_mac_code("${_file}")
  endif()
endforeach()

# Seal framework resources after their code, then seal the enclosing app last.
file(GLOB _frameworks LIST_DIRECTORIES TRUE "${_app}/Contents/Frameworks/*.framework")
foreach(_framework IN LISTS _frameworks)
  yh_sign_mac_code("${_framework}")
endforeach()
yh_sign_mac_code("${_app}")
execute_process(COMMAND /usr/bin/codesign --verify --deep --strict --verbose=2 "${_app}"
  RESULT_VARIABLE _result OUTPUT_VARIABLE _output ERROR_VARIABLE _error)
if(NOT "${_result}" STREQUAL "0")
  message(FATAL_ERROR "Deployed app signature is invalid: ${_result}\n${_output}${_error}")
endif()
message(STATUS "Signed and verified deployed app: ${_app}")
