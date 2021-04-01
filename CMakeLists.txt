idf_component_register(
    SRCS
        "epaper-idf-gfx.cpp"
        "epaper-idf-io.cpp"
        "epaper-idf-spi.cpp"

        "device/Gdew075T8.cpp"

    REQUIRES
        "Adafruit-GFX-Component"

    INCLUDE_DIRS
        "include"
)

add_compile_definitions(
    ESP32_IDF
)
