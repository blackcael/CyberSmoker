#include "thermometer.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "hw.h"

#define TAG "THERMOMETERS"

// ADC channel assignments
#define MEAT_ADC_CHANNEL    ADC_CHANNEL_3
#define AMBIENT_ADC_CHANNEL ADC_CHANNEL_4

// ADC config
#define ADC_UNIT_ID         ADC_UNIT_1
#define ADC_MAX_READING     4095.0
#define ADC_REF_VOLTAGE     3.3  // Adjust if using internal calibration
#define ADC_SCALER          100

// ADC handles
static adc_oneshot_unit_handle_t adc_handle;

// Channel identifiers (optional for reuse)
static adc_channel_t meat_channel = MEAT_ADC_CHANNEL;
static adc_channel_t ambient_channel = AMBIENT_ADC_CHANNEL;

void thermometer_init() {
    // 1. Create ADC unit
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_ID,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    // 2. Configure both channels
    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,  // 0-3.3V range
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, meat_channel, &chan_config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ambient_channel, &chan_config));
    
    

    ESP_LOGI(TAG, "Thermometers initialized");
}

temperature_t thermometer_read_meat() {
    int raw = 0;
    esp_err_t err = adc_oneshot_read(adc_handle, meat_channel, &raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read meat thermometer: %s", esp_err_to_name(err));
        return -1.0;
    }

    float voltage = (raw / ADC_MAX_READING) * ADC_REF_VOLTAGE * ADC_SCALER;
    return (temperature_t)voltage;
}

temperature_t thermometer_read_ambient() {
    int raw = 0;
    esp_err_t err = adc_oneshot_read(adc_handle, ambient_channel, &raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read ambient thermometer: %s", esp_err_to_name(err));
        return -1.0;
    }

    float voltage = (raw / ADC_MAX_READING) * ADC_REF_VOLTAGE * ADC_SCALER;
    return (temperature_t)voltage;
}