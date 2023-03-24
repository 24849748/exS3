set(EXS3_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE SOURCES
    ${EXS3_ROOT_DIR}/*.c
    ${EXS3_ROOT_DIR}/bus/*.c 
    ${EXS3_ROOT_DIR}/lcd/*.c 
    ${EXS3_ROOT_DIR}/touch/*.c 
    ${EXS3_ROOT_DIR}/power/*c
    ${EXS3_ROOT_DIR}/iodev/*c
    ${EXS3_ROOT_DIR}/encoder/*c
    ${EXS3_ROOT_DIR}/button/*c
)

idf_component_register(
    SRCS
        ${SOURCES}
    INCLUDE_DIRS
        ${EXS3_ROOT_DIR}
        ${EXS3_ROOT_DIR}/bus
        ${EXS3_ROOT_DIR}/lcd
        ${EXS3_ROOT_DIR}/touch
        ${EXS3_ROOT_DIR}/power
        ${EXS3_ROOT_DIR}/iodev
        ${EXS3_ROOT_DIR}/encoder
        ${EXS3_ROOT_DIR}/button

    REQUIRES
        driver lvgl 
)