idf_component_register(
  SRCS
    "epaper-idf-gfx.cpp"
    "epaper-idf-http.c"
    "epaper-idf-io.cpp"
    "epaper-idf-ota.c"
    "epaper-idf-spi.cpp"
    "epaper-idf-wifi.c"

    "device/Gdew075T8.cpp"

    "task/http-slideshow.cpp"

    $ENV{IDF_PATH}/examples/common_components/protocol_examples_common/connect.c
    $ENV{IDF_PATH}/examples/common_components/protocol_examples_common/addr_from_stdin.c
    $ENV{IDF_PATH}/examples/common_components/protocol_examples_common/stdin_out.c

    $ENV{IDF_PATH}/components/esp_https_ota/src/esp_https_ota.c

  REQUIRES
    "Adafruit-GFX-Component"

  INCLUDE_DIRS
    "include"

    $ENV{IDF_PATH}/components/app_update/include
    $ENV{IDF_PATH}/components/bootloader_support/include
    $ENV{IDF_PATH}/components/esp_http_client/include
    $ENV{IDF_PATH}/components/esp_https_ota/include
    $ENV{IDF_PATH}/components/mbedtls/esp_crt_bundle/include
    $ENV{IDF_PATH}/components/mbedtls/mbedtls/include
    $ENV{IDF_PATH}/components/mbedtls/port/include
    $ENV{IDF_PATH}/components/nghttp/port/include
    $ENV{IDF_PATH}/components/nvs_flash/include
    $ENV{IDF_PATH}/components/spi_flash/include
    $ENV{IDF_PATH}/components/ulp/include
    $ENV{IDF_PATH}/components/esp-tls
    $ENV{IDF_PATH}/components/esp_wifi/include

    $ENV{IDF_PATH}/examples/common_components/protocol_examples_common/include

  # Embed the server root certificate into the final binary
  EMBED_TXTFILES 
    ${project_dir}/certs/ca_cert.pem
)

# (Not part of the boilerplate)
# This example uses an extra component for common functions such as Wi-Fi and Ethernet connection.
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

add_compile_definitions(
  ESP32_IDF
)
