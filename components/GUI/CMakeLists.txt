set(GUI_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE SOURCES 
        ${GUI_ROOT_DIR}/app/common/*.c
        ${GUI_ROOT_DIR}/app/font/*.c
        ${GUI_ROOT_DIR}/app/page/*.c
        ${GUI_ROOT_DIR}/app/img/*.c
        ${GUI_ROOT_DIR}/app/anim/*.c
        ${GUI_ROOT_DIR}/app/widgets/*.c
        ${GUI_ROOT_DIR}/port/*.c
        )


idf_component_register(
    SRCS 
        ${SOURCES} 
    INCLUDE_DIRS
        ${GUI_ROOT_DIR}/port
        ${GUI_ROOT_DIR}/app/common
        ${GUI_ROOT_DIR}/app/page
        ${GUI_ROOT_DIR}/app/anim
        ${GUI_ROOT_DIR}/app/widgets
    REQUIRES 
        lvgl lvgl_drivers widgets axp173 spiffs)