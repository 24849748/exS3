idf_component_register(SRCS "i2c_bus.c" 
                            "i2c_mid.c"
                            "spi_bus.c"
                        INCLUDE_DIRS "."
                        REQUIRES driver)