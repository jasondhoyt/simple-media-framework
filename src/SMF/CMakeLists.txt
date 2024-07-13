add_library(SMF)

target_sources(SMF
    PRIVATE
        SMF_context.c
        SMF_mem.c
        SMF_window.c
)

target_include_directories(SMF
    PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(SMF
    PRIVATE
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
