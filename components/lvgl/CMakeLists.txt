cmake_minimum_required(VERSION 3.12.4)

set(LVGL_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE SOURCES ${LVGL_ROOT_DIR}/src/*.c 
                          ${LVGL_ROOT_DIR}/demos/*.c)

idf_build_get_property(LV_MICROPYTHON LV_MICROPYTHON)


idf_component_register(
    SRCS 
      ${SOURCES} 
    INCLUDE_DIRS 
      ${LVGL_ROOT_DIR}
      ${LVGL_ROOT_DIR}/src 
      ${LVGL_ROOT_DIR}/demos
      ${LVGL_ROOT_DIR}/../)

target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_CONF_INCLUDE_SIMPLE")

# IRAM 相关
#if(CONFIG_LV_ATTRIBUTE_FAST_MEM_USE_IRAM)
#  target_compile_definitions(${COMPONENT_LIB}
#                           PUBLIC "-DLV_ATTRIBUTE_FAST_MEM=IRAM_ATTR")
#endif()

#LVGL version 8.3.1

