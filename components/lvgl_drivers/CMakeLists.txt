set(LV_DRV_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE SOURCES 
        ${LV_DRV_ROOT_DIR}/tft/*.c
        ${LV_DRV_ROOT_DIR}/touch/*.c
        )


idf_component_register(
    SRCS 
        ${SOURCES} 
    INCLUDE_DIRS
        ${LV_DRV_ROOT_DIR}/touch
        ${LV_DRV_ROOT_DIR}/tft
        # ${LV_DRV_ROOT_DIR}
    REQUIRES 
        lvgl bus driver)