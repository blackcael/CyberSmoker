#include <stdio.h>

#include "mqtt_client.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"

#include "cJSON.h"

#include "smoker_fsm.h"

static const char *TAG = "MAIN";

void app_main(void) {
    smoker_fsm_init();
    smoker_fsm_run();
}