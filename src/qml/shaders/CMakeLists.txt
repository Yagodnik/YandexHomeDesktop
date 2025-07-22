set(SHADER_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/highlight.frag"
)

set(SHADER_OUT
        "${CMAKE_SOURCE_DIR}/resources/shaders/highlight.frag.qsb"
)

add_custom_command(
        OUTPUT ${SHADER_OUT}
        COMMAND qsb
        "${SHADER_SRC}"
        -o "${SHADER_OUT}"
        --glsl 310es
        --hlsl 50
        --msl 21
        DEPENDS ${SHADER_SRC}
        COMMENT "Compiling shader ${SHADER_SRC} to ${SHADER_OUT}"
        VERBATIM
)

add_custom_target(compile_shaders ALL DEPENDS ${SHADER_OUT})