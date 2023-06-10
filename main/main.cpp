// standard C libraries
#include <stdio.h>

// esp-idf libraries
#include "esp_log.h"
#include "esp_timer.h"

// esp-idf drivers
#include "driver/i2c.h"

#include "SerLCD.h"

static const char *TAG = "SerLCD example";

#define I2C_CLIENT_SCL_IO  GPIO_NUM_16 /*!< GPIO number used for I2C client clock */
#define I2C_CLIENT_SDA_IO  GPIO_NUM_13 /*!< GPIO number used for I2C client data  */
i2c_port_t i2c_client_num = 1;                        /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
// TEST #define I2C_CLIENT_FREQ_HZ 400000               /*!< I2C master clock frequency */
#define I2C_CLIENT_FREQ_HZ 50000               /*!< I2C master clock frequency */ //320000 too fast for the AIP display
#define I2C_CLIENT_TX_BUF_DISABLE 0             /*!< I2C master doesn't need buffer */
#define I2C_CLIENT_RX_BUF_DISABLE 0             /*!< I2C master doesn't need buffer */
#define I2C_CLIENT_TIMEOUT_MS 1000

SerLCD lcd; // Initialize the library with default I2C address 0x72


/**
 * @brief i2c client initialization
 */
static esp_err_t i2c_client_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_CLIENT_SDA_IO,
        .scl_io_num = I2C_CLIENT_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE, //.sda_pullup_en
        .scl_pullup_en = GPIO_PULLUP_DISABLE, //.scl_pullup_en
        .master = {.clk_speed = I2C_CLIENT_FREQ_HZ}, //.master.clk_speed
        .clk_flags = 0//  =  default clock TEST
    };

    ESP_ERROR_CHECK(i2c_param_config(i2c_client_num, &conf));

    return i2c_driver_install(i2c_client_num, conf.mode, I2C_CLIENT_RX_BUF_DISABLE, I2C_CLIENT_TX_BUF_DISABLE, 0);
}

extern "C" void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_DEBUG); // set all components to DEBUG level
    ESP_ERROR_CHECK(i2c_client_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    ESP_LOGI(TAG, "portTICK_PERIOD_MS: %li", portTICK_PERIOD_MS);

  lcd.begin(i2c_client_num); //Set up the LCD for I2C communication

  lcd.setBacklight(255, 255, 255); //Set backlight to bright white
  lcd.setContrast(5); //Set contrast. Lower to 0 for higher contrast.

  lcd.clear(); //Clear the display - this moves the cursor to home position as well
  lcd.print("Hello, World!");
    while (true){
        // Set the cursor to column 0, line 1
        // (note: line 1 is the second row, since counting begins with 0):
        lcd.setCursor(0, 1);
        // Print the number of seconds since reset:
        lcd.print((uint32_t)(esp_timer_get_time() / 1000000));
    }
}