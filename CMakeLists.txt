idf_component_register(
  SRCS
    "epaper-idf-gfx.cpp"
    "epaper-idf-io.cpp"
    "epaper-idf-ota.c"
    "epaper-idf-spi.cpp"

    "device/Gdew075T8.cpp"

    $ENV{IDF_PATH}/components/esp_https_ota/src/esp_https_ota.c

  REQUIRES
    "Adafruit-GFX-Component"

  INCLUDE_DIRS
    "include"

    $ENV{IDF_PATH}/components/app_update/include
    $ENV{IDF_PATH}/components/bootloader_support/include
    $ENV{IDF_PATH}/components/esp_http_client/include
    $ENV{IDF_PATH}/components/esp_https_ota/include
    $ENV{IDF_PATH}/components/nghttp/port/include
    $ENV{IDF_PATH}/components/nvs_flash/include
    $ENV{IDF_PATH}/components/spi_flash/include

  # Embed the server root certificate into the final binary
  EMBED_TXTFILES 
    ${project_dir}/certs/ca_cert.pem
)

add_compile_definitions(
  ESP32_IDF
)
