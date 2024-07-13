add_executable(test_app)

target_sources(test_app
    PRIVATE
        main.c
)

target_include_directories(test_app
    PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(test_app
    PRIVATE
        SMF
)
