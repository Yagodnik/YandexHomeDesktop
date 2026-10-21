if(YH_ENABLE_PCH)
  foreach(_pch_target YandexApiAdapter Models Utils Iot appYandexHomeDesktop
      SerializationTests ApiTests ApiBoundaryTests ModelBoundaryTests CapabilitiesTests ModelsTests)
    if(TARGET ${_pch_target})
      target_precompile_headers(${_pch_target} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/serialization/PrecompiledHeaders.h")
    endif()
  endforeach()
endif()
