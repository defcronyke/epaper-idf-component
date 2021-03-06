/**	epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.

		Contains some modified example code from here:
		https://github.com/espressif/esp-idf/blob/release/v4.2/examples/system/ota/advanced_https_ota/main/advanced_https_ota_example.c

		Original Example Code Header:
		This code is in the Public Domain (or CC0 licensed, at your option.)

		Unless required by applicable law or agreed to in writing, this
		software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
		CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"

#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

#include "lwip/inet.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"

#include "protocol_examples_common.h"

#include "epaper-idf-wifi.h"

const char *epaper_idf_wifi_tag = "epaper-idf-wifi";

// NOTE: For wifi captive portal whenever someone makes an ESP-IDF DNS server.
// static const char epaper_idf_wifi_hostname[] = CONFIG_LWIP_LOCAL_HOSTNAME;
// static ip_addr_t epaper_idf_wifi_ip;

esp_event_loop_handle_t epaper_idf_wifi_event_loop_handle;
static struct epaper_idf_wifi_task_action_value_t wifi_task_action_value;
static struct epaper_idf_wifi_task_action_t wifi_task_action;

ESP_EVENT_DEFINE_BASE(EPAPER_IDF_WIFI_EVENT);

#ifdef CONFIG_EXAMPLE_WIFI_AP_STARTUP_CONNECTION_RETRIES_OPT
static const int retries = CONFIG_EXAMPLE_WIFI_AP_STARTUP_CONNECTION_RETRIES;
#else
static const int retries = CONFIG_EXAMPLE_WIFI_CONNECTION_RETRIES;
#endif

#ifdef CONFIG_EXAMPLE_CONNECT_WIFI

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t wifi_event_group;
static int retry_num = 0;

static bool try_reconnect = true;

#endif

#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    // BaseType_t stack_res = uxTaskGetStackHighWaterMark(NULL);
    // ESP_LOGW(epaper_idf_wifi_tag, "!!! [ wifi task ] before: esp_wifi_connect() -> STACK SIZE !!!: %d", stack_res);

    ESP_LOGI(epaper_idf_wifi_tag, "attempting to connect to wifi network...");
    esp_wifi_connect();

    // stack_res = uxTaskGetStackHighWaterMark(NULL);
    // ESP_LOGW(epaper_idf_wifi_tag, "!!! [ wifi task ] after: esp_wifi_connect() -> STACK SIZE !!!: %d", stack_res);
  }
  else if (try_reconnect && event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    if (retry_num < retries)
    {
      ESP_LOGI(epaper_idf_wifi_tag, "attempting to connect to wifi network...");
      esp_wifi_connect();
      retry_num++;
      ESP_LOGI(epaper_idf_wifi_tag, "retry to connect to the AP");
    }
    else
    {
      xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGI(epaper_idf_wifi_tag, "connect to the AP fail");
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(epaper_idf_wifi_tag, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    retry_num = 0;
    xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);

    // BaseType_t stack_res = uxTaskGetStackHighWaterMark(NULL);
    // ESP_LOGW(epaper_idf_wifi_tag, "!!! [ wifi task ] after: IP_EVENT_STA_GOT_IP -> STACK SIZE !!!: %d", stack_res);
  }
  else if (event_id == WIFI_EVENT_AP_STACONNECTED)
  {
    wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(epaper_idf_wifi_tag, "station " MACSTR " join, AID=%d",
             MAC2STR(event->mac), event->aid);

    // BaseType_t stack_res = uxTaskGetStackHighWaterMark(NULL);
    // ESP_LOGW(epaper_idf_wifi_tag, "!!! [ wifi task ] after: WIFI_EVENT_AP_STACONNECTED -> STACK SIZE !!!: %d", stack_res);
  }
  else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
  {
    wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(epaper_idf_wifi_tag, "station " MACSTR " leave, AID=%d",
             MAC2STR(event->mac), event->aid);

    // BaseType_t stack_res = uxTaskGetStackHighWaterMark(NULL);
    // ESP_LOGW(epaper_idf_wifi_tag, "!!! [ wifi task ] after: WIFI_EVENT_AP_STADISCONNECTED -> STACK SIZE !!!: %d", stack_res);
  }
}
#endif

#ifdef CONFIG_EXAMPLE_WIFI_AP_ENABLED
static void epaper_idf_wifi_ap_init(void)
{
  wifi_config_t wifi_config_ap = {
      .ap = {
          .ssid = CONFIG_EXAMPLE_WIFI_AP_SSID,
          .ssid_len = strlen(CONFIG_EXAMPLE_WIFI_AP_SSID),
          .channel = CONFIG_EXAMPLE_WIFI_AP_CHANNEL,
          .password = CONFIG_EXAMPLE_WIFI_AP_PASSWORD,
          .max_connection = CONFIG_EXAMPLE_MAX_AP_CONN,
          .authmode = WIFI_AUTH_WPA_WPA2_PSK},
  };
  if (strlen(CONFIG_EXAMPLE_WIFI_AP_PASSWORD) == 0)
  {
    wifi_config_ap.ap.authmode = WIFI_AUTH_OPEN;
  }

#ifdef CONFIG_EXAMPLE_WIFI_AP_STARTUP_ALWAYS_ON_OPT
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
#else
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
#endif

  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config_ap));

  ESP_LOGI(epaper_idf_wifi_tag, "starting WiFi access point: SSID: %s password:%s channel: %d",
           CONFIG_EXAMPLE_WIFI_AP_SSID, CONFIG_EXAMPLE_WIFI_AP_PASSWORD, CONFIG_EXAMPLE_WIFI_AP_CHANNEL);

  // BaseType_t stack_res = uxTaskGetStackHighWaterMark(NULL);
  // ESP_LOGW(epaper_idf_wifi_tag, "!!! [ wifi task ] before: epaper_idf_dns_init() -> STACK SIZE !!!: %d", stack_res);

  /** NOTE: Wifi captive portal. */
  epaper_idf_dns_init();

  // stack_res = uxTaskGetStackHighWaterMark(NULL);
  // ESP_LOGW(epaper_idf_wifi_tag, "!!! [ wifi task ] after: epaper_idf_dns_init() -> STACK SIZE !!!: %d", stack_res);
}
#endif /**< End CONFIG_EXAMPLE_WIFI_AP_ENABLED */

static void epaper_idf_wifi_init(void)
{
  /** Initialize NVS. */
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
    // partition table. This size mismatch may cause NVS initialization to fail.
    // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
    // If this happens, we erase NVS partition and initialize NVS again.
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  ESP_ERROR_CHECK(esp_netif_init());

#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
  wifi_event_group = xEventGroupCreate();

  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
  assert(ap_netif);

  esp_netif_ip_info_t ip_info;
  // TODO: There is a value "CONFIG_EXAMPLE_WIFI_AP_IP" in the
  // Kconfig menu which isn't actually used here, but it should
  // be.
  IP4_ADDR(&ip_info.ip, 126, 233, 53, 78);
  IP4_ADDR(&ip_info.gw, 126, 233, 53, 78);
  IP4_ADDR(&ip_info.netmask, 255, 0, 0, 0);
  esp_netif_dhcps_stop(ap_netif);
  esp_netif_set_ip_info(ap_netif, &ip_info);
  esp_netif_dhcps_start(ap_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

#ifdef CONFIG_EXAMPLE_WIFI_AP_STARTUP_ALWAYS_ON_OPT
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  epaper_idf_wifi_ap_init();
#else
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
#endif

  wifi_config_t wifi_config_sta = {
      .sta = {
          .ssid = CONFIG_EXAMPLE_WIFI_SSID,
          .channel = CONFIG_EXAMPLE_WIFI_CHANNEL,
          .password = CONFIG_EXAMPLE_WIFI_PASSWORD},
  };

#ifndef CONFIG_EXAMPLE_WIFI_AP_STARTUP_ALWAYS_ON_OPT
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
#endif

  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config_sta));

  esp_event_handler_instance_t instance_any_id;

  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(
          WIFI_EVENT,
          ESP_EVENT_ANY_ID,
          &wifi_event_handler,
          NULL,
          &instance_any_id));

  ESP_ERROR_CHECK(esp_wifi_start());

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE,
                                         pdFALSE,
                                         portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
    	happened. */
  if (bits & WIFI_CONNECTED_BIT)
  {
    ESP_LOGI(epaper_idf_wifi_tag, "connected to ap SSID: %s password: %s",
             CONFIG_EXAMPLE_WIFI_SSID, CONFIG_EXAMPLE_WIFI_PASSWORD);
  }
  else if (bits & WIFI_FAIL_BIT)
  {
    ESP_LOGI(epaper_idf_wifi_tag, "Failed to connect to SSID: %s, password: %s",
             CONFIG_EXAMPLE_WIFI_SSID, CONFIG_EXAMPLE_WIFI_PASSWORD);

    // BaseType_t stack_res = uxTaskGetStackHighWaterMark(NULL);
    // ESP_LOGW(epaper_idf_wifi_tag, "!!! [ wifi task ] after: WIFI_CONNECTED_BIT || WIFI_FAIL_BIT -> STACK SIZE !!!: %d", stack_res);

#ifdef CONFIG_EXAMPLE_WIFI_AP_ENABLED
    /** Start the WiFi access point (AP) if it's configured to 
					start after a certain number of connection retries. The 
					access point can be used to configure which WiFi network 
					to connect to. */
    ESP_LOGI(epaper_idf_wifi_tag, "starting WiFi access point after %d attempts", retry_num);

#ifndef CONFIG_EXAMPLE_WIFI_AP_STARTUP_ALWAYS_ON_OPT
    ESP_ERROR_CHECK(esp_wifi_stop());

    epaper_idf_wifi_ap_init();

    ESP_ERROR_CHECK(esp_wifi_start());
#endif
#endif
  }
  else
  {
    ESP_LOGE(epaper_idf_wifi_tag, "UNEXPECTED EVENT");
  }

  // /* The event will not be processed after unregister */
  // // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
  // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
  // vEventGroupDelete(wifi_event_group);

  // // Scan for other WIFI networks.
  // char *wifi_scan_res = NULL;
  // size_t wifi_scan_res_len = wifi_scan(&wifi_scan_res);

  // if (wifi_scan_res_len > 0)
  // {
  // 	ESP_LOGI(epaper_idf_wifi_tag, "%s\n", wifi_scan_res);
  // 	free(wifi_scan_res);
  // }

#endif /**< End CONFIG_EXAMPLE_CONNECT_WIFI */
}

#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
static void epaper_idf_wifi_disconnect(void)
{
  ESP_ERROR_CHECK(esp_wifi_disconnect());
}
#endif

/** The WiFi task. */
void epaper_idf_wifi_task(void *pvParameter)
{
  while (1)
  {
    // while (1)
    // {
    bool wifi_is_init = false;
    wifi_task_action = EPAPER_IDF_WIFI_TASK_ACTION_COPY(pvParameter);

    esp_err_t err = ESP_OK;

    switch (wifi_task_action.id)
    {
    /** Initialize (if necessary), and connect to a wifi network. */
    case EPAPER_IDF_WIFI_TASK_ACTION_CONNECT:
    {
      ESP_LOGI(epaper_idf_wifi_tag, "running epaper idf wifi task action: EPAPER_IDF_WIFI_TASK_ACTION_CONNECT");

      /** Init only once. */
      if (!wifi_is_init)
      {
#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
        try_reconnect = true;

        ESP_LOGI(epaper_idf_wifi_tag, "wifi initializing...");
#endif

        /** Initialize wifi (or just ethernet, but that's not 
						currently supported, so things won't work properly
						with wifi disabled). */
        epaper_idf_wifi_init();

        wifi_is_init = true;
      }

      break;
    }

    /** Disconnect wifi interface. */
    case EPAPER_IDF_WIFI_TASK_ACTION_DISCONNECT:
    {
      ESP_LOGI(epaper_idf_wifi_tag, "running epaper idf wifi task action: EPAPER_IDF_WIFI_TASK_ACTION_DISCONNECT");

#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
      /** Disconnect wifi. */
      epaper_idf_wifi_disconnect();
#endif

      break;
    }

    /** Disconnect and stop wifi interface. */
    case EPAPER_IDF_WIFI_TASK_ACTION_STOP:
    {
      ESP_LOGI(epaper_idf_wifi_tag, "running epaper idf wifi task action: EPAPER_IDF_WIFI_TASK_ACTION_STOP");

      if (wifi_task_action.value != NULL)
      {
        wifi_task_action_value = EPAPER_IDF_WIFI_TASK_ACTION_VALUE_COPY(wifi_task_action.value);
      }

#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
      try_reconnect = false;

      /** Disconnect wifi. */
      epaper_idf_wifi_disconnect();

      /** Stop wifi interface. */
      esp_wifi_stop();
#endif

      /** Send an event which says "this task is finished". */
      err = esp_event_post_to(epaper_idf_wifi_event_loop_handle, EPAPER_IDF_WIFI_EVENT, EPAPER_IDF_WIFI_EVENT_STOPPED, EPAPER_IDF_WIFI_TASK_ACTION_VALUE_CAST_VOID_P(wifi_task_action_value), sizeof(wifi_task_action_value), portMAX_DELAY);
      if (err != ESP_OK)
      {
        ESP_LOGE(epaper_idf_wifi_tag, "Sending event failed");
      }

      break;
    }

    default:
    {
      break;
    }
    }

    /** Send an event which says "this task is finished". */
    err = esp_event_post_to(epaper_idf_wifi_event_loop_handle, EPAPER_IDF_WIFI_EVENT, EPAPER_IDF_WIFI_EVENT_FINISH, NULL, 0, portMAX_DELAY);
    if (err != ESP_OK)
    {
      ESP_LOGE(epaper_idf_wifi_tag, "Sending event failed");
    }

    // break;
    // }

    vTaskDelete(NULL);

    break;
  }
}
