#include "HX711.h"
#include <HardwareSerial.h>

HardwareSerial MySerial(1);

// HX711
#define HX_DT 19
#define HX_SCK 18
#define HX_VCC_CTRL 23

// SigFox UART (UART2)
#define SIGFOX_TX 16
#define SIGFOX_RX 17
#define SIGFOX_BAUD 9600

// Capteur anti-vol
#define GPIO_WAKEUP 33
char idStr[9] = ""; // 8 chars + null
int i = 0;

HX711 scale;

void setup() {
  Serial.begin(115200);
  MySerial.begin(9600, SERIAL_8N1, 17, 16); // serial port to send RTCM to F9P
    delay(100);
  // Alimentation du HX711 via GPIO
  pinMode(HX_VCC_CTRL, OUTPUT);
  digitalWrite(HX_VCC_CTRL, HIGH);
  delay(100);  // Laisser le capteur démarrer

  // Initialisation HX711
  scale.begin(HX_DT, HX_SCK);
  scale.set_scale(48.182);  // Ajuste selon ton calibrage
  //scale.tare();
  // Faire la tare uniquement au tout premier démarrage (alimentation)
if (esp_reset_reason() == ESP_RST_POWERON) {
  scale.tare();
}
  delay(100);

  MySerial.println("AT$I=10");
  delay(300);
  
  // Lecture poids
  float poids_g = scale.get_units(10);
  if (poids_g < 0) poids_g = 0;
  uint32_t poids10 = (uint32_t)(poids_g * 10);
  Serial.println(poids10);
  // Créer payload (little-endian)
  byte payload[4];
  payload[0] = poids10 & 0xFF;
  payload[1] = (poids10 >> 8) & 0xFF;
  payload[2] = (poids10 >> 16) & 0xFF;
  payload[3] = (poids10 >> 24) & 0xFF;
  
  // Construire payload hex pour AT$SF
  char hexStr[9];
  sprintf(hexStr, "%02X%02X%02X%02X",
    payload[0], payload[1], payload[2], payload[3]);
  
  MySerial.print("AT$SF=");
  MySerial.println(hexStr);
  delay(1500);

  // Coupure du capteur pour l'économie d'énergie
  digitalWrite(HX_VCC_CTRL, LOW);

  // Configuration du réveil par interruption GPIO33 (anti-vol)
  //pinMode(GPIO_WAKEUP, INPUT_PULLUP); // bouton/contact = niveau bas = alerte
  //esp_sleep_enable_ext0_wakeup((gpio_num_t)GPIO_WAKEUP, 0); // 0 = niveau bas déclencheur

  // Réveil automatique
  //esp_sleep_enable_timer_wakeup(3ULL * 60ULL * 60ULL * 1000000ULL);  // 3 heures
  esp_sleep_enable_timer_wakeup(1800ULL * 1000000ULL);  // 30 minutes
  //esp_sleep_enable_timer_wakeup(600ULL * 1000000ULL);  // 10 minutes
  
  esp_deep_sleep_start();
}

void loop() { }
