///@file app_main.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Starting point of project
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

#include <stdio.h>
#include <string.h>

#include "../components/library/include/commands_registration.h"
#include "../components/library/parameters.h"
#include "ascii_art.c"
#include "cmd_nvs.h"
#include "cmd_system.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"
#include "internet_check.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "wifi.h"

static const char *TAG = "app_main.c";

typedef enum DataTypes_e {
  type_char,
  type_int,
  type_float,
} DataTypes_t;

static void initialize_nvs(void) {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

void app_main(void) {
  // Plant Protector !!!
  PrintAsciiArt();

  // Initialize NVS
  initialize_nvs();

  printf("Registering commands...");
  register_commands();
  printf("Done\n");

  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

  char ssid[32] = {0};
  char password[64] = {0};

  int res =
      load_wifi_credentials(ssid, sizeof(ssid), password, sizeof(password));
  if (res == EXIT_FAILURE) {
    printf("SAVING SSID AND PASSWORD TO NVS\n");
    strncpy(ssid, DEFAULT_SSID, sizeof(ssid));
    strncpy(password, DEFAULT_PASS, sizeof(password));
    save_wifi_credentials(ssid, password);
  } else {
    printf("LOADED SSID AND PASS from NVS!!\n");
  }

  wifi_init_sta(ssid, password);
  check_internet_connection();

  printf("---CMD SHOULD BE INTERACTIVE FROM HERE\n");
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  repl_config.prompt = "PlantProtector>";
  repl_config.max_cmdline_length = 150;

  /* Register commands */
  esp_console_register_help_command();
  register_system();
  register_nvs();

  esp_console_dev_uart_config_t hw_config =
      ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}