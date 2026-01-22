#include "wifi_wrapper.h"

#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_attr.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "cJSON.h"

#define WIFI_SSID "crack_house"
#define WIFI_PASS "chingchong"
#define BROKER_URI "mqtt://192.168.1.230"

#include "freertos/event_groups.h"

#define WIFI_CONNECTED_BIT BIT0

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static EventGroupHandle_t s_wifi_event_group;

esp_mqtt_client_handle_t client;

static const char *TAG = "WIFI_WRAPPER";

static bool wifi_wrapper_connected = false;

void (*turn_on_cmd)(void) = NULL;
void (*turn_off_cmd)(void) = NULL;
void (*inc_temp_cmd)(void) = NULL;
void (*dec_temp_cmd)(void) = NULL;


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_publish(event->client, "esp32/test", "hello from ESP32", 0, 1, 0);
            esp_mqtt_client_subscribe(event->client, "esp32/command", 0);
            wifi_wrapper_connected = true;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            wifi_wrapper_connected = false;
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Received topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "Received data: %.*s", event->data_len, event->data);

            // Compare topic
            if (strncmp(event->topic, "esp32/command", event->topic_len) == 0) {
                // Copy data into a null-terminated string
                char cmd[64] = {0};
                strncpy(cmd, event->data, MIN(event->data_len, sizeof(cmd) - 1));
            
                // Handle commands
                if (strcmp(cmd, "ON") == 0) {
                    ESP_LOGI(TAG, "Command: TURN ON");
                    if(turn_on_cmd != NULL){
                        turn_on_cmd();
                    }else{
                        ESP_LOGI(TAG, "No Command is Registered!");
                    } 
                } else if (strcmp(cmd, "OFF") == 0) {
                    ESP_LOGI(TAG, "Command: TURN OFF");
                    if(turn_off_cmd != NULL){
                        turn_off_cmd();
                    }else{
                        ESP_LOGI(TAG, "No Command is Registered!");
                    }
                } else if (strcmp(cmd, "set_temp_inc") == 0) {
                    ESP_LOGI(TAG, "Command: Increase Temp");
                    if(inc_temp_cmd != NULL){
                        inc_temp_cmd();
                    }else{
                        ESP_LOGI(TAG, "No Command is Registered!");
                    }
                } else if (strcmp(cmd, "set_temp_dec") == 0) {
                    ESP_LOGI(TAG, "Command: Decrease Temp");
                    if(dec_temp_cmd != NULL){
                        dec_temp_cmd();
                    }else{
                        ESP_LOGI(TAG, "No Command is Registered!");
                    }
                }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
        }
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();  // auto-reconnect
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("WIFI", "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void publish_smoker_data(esp_mqtt_client_handle_t client, temperature_t meat_temp, temperature_t ambient_temp, temperature_t set_temp, uint8_t pellet_level) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "meat_temp", (uint16_t)meat_temp);
    cJSON_AddNumberToObject(root, "ambient_temp", (uint16_t)ambient_temp);
    cJSON_AddNumberToObject(root, "set_temp", (uint16_t)set_temp);
    cJSON_AddNumberToObject(root, "pellet_level", pellet_level);

    char *json_str = cJSON_PrintUnformatted(root);
    esp_mqtt_client_publish(client, "esp32/data/smoker_data", json_str, 0, 1, 0);
    cJSON_Delete(root);
    free(json_str);
}

void wifi_wrapper_init(){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    // Wait until connected before proceeding
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);

    // Now it's safe to start MQTT
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = BROKER_URI,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    // Optional: publish on startup
    publish_smoker_data(client, 180, 220, 225, 67);
}

void wifi_wrapper_publish_data(temperature_t meat_temp, temperature_t ambient_temp, temperature_t set_temp, uint8_t pellet_level){
    publish_smoker_data(client, meat_temp, ambient_temp, set_temp, pellet_level);
}

void wifi_wrapper_register_turn_off_cmd(void(*cmd)(void)){
    turn_off_cmd = cmd;
}

void wifi_wrapper_register_turn_on_cmd(void(*cmd)(void)){
    turn_on_cmd = cmd;
}

void wifi_wrapper_register_inc_temp_cmd(void(*cmd)(void)){
    inc_temp_cmd = cmd;
}

void wifi_wrapper_register_dec_temp_cmd(void(*cmd)(void)){
    dec_temp_cmd = cmd;
}

bool wifi_wrapper_get_connection_status(void){
    return wifi_wrapper_connected;
}