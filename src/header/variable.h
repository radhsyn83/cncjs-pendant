// EEPROM
#define EEPROM_SIZE 100
//  WIFI
#define WIFI_AP_SSID "Pendant CNCJS"
#define WIFI_AP_PASSWORD "12345678"
// MQTT
#define MQTT_HOST "192.168.1.137"
#define MQTT_PORT 1883
#define MQTT_USERNAME "syn"
#define MQTT_PASSWORD "sense324"
#define MQTT_TOPIC_PUBLISH_TO "cncjs"
#define MQTT_TOPIC_RECEIVE_FROM "esp32"
#define JSON_DOC_SIZE 1024
// ST7789 DISPLAY
#define TFT_CS -1         // Chip select control pin
#define TFT_DC 2          // Data Command control pin
#define TFT_RST 4         // Reset pin (could connect to RST pin)
#define BACKLIGHT_PIN 15  // Example pin for controlling backlight
#define SCREEN_SIZE 240
#define MAX_LOG_LINES 30
#define SHOW_LOG_ON 1  // 0: both, 1: sceen, 2: serial
