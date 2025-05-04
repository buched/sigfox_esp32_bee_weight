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

// Capteur
//#define SENSOR_ID 260

HX711 scale;

void setup() {
  Serial.begin(115200);
  //Serial2.begin(SIGFOX_BAUD, SERIAL_8N1, SIGFOX_RX, SIGFOX_TX);
  MySerial.begin(9600, SERIAL_8N1, 16, 17); // serial port to send RTCM to F9P
    delay(100);
  // Alimentation du HX711 via GPIO
  pinMode(HX_VCC_CTRL, OUTPUT);
  digitalWrite(HX_VCC_CTRL, HIGH);
  delay(100);  // Laisser le capteur démarrer

  // Initialisation HX711
  scale.begin(HX_DT, HX_SCK);
  scale.set_scale(41.1);  // Ajuste selon ton calibrage
  scale.tare();
  delay(100);
//
//  uint16_t id = 260;         // ID capteur
//  // Lecture du poids
//  float poids_g=scale.get_units(10);
//  //poids_g = 28.6789;
//  if (poids_g < 0) poids_g = 0;
//  uint16_t poids10 = (uint16_t)(poids_g * 10);  // Poids avec 1 décimale
//
//  // Little-endian : LSB puis MSB
//  byte payload[4];
//  payload[0] = id & 0xFF;
//  payload[1] = (id >> 8) & 0xFF;
//  payload[2] = poids10 & 0xFF;
//  payload[3] = (poids10 >> 8) & 0xFF;
//
//  // Convertir en chaîne hex pour AT$SF
//  char hexStr[9]; // 4 octets = 8 caractères + null
//  sprintf(hexStr, "%02X%02X%02X%02X", payload[0], payload[1], payload[2], payload[3]);
//
//  //Serial.print("Payload HEX (little-endian) : ");
//  //Serial.println(hexStr); // ex: 4300DE14
//
//  MySerial.print("AT$SF=");
//  MySerial.println(hexStr);
// Lire l'ID SigFox depuis le module
char idStr[9] = ""; // 8 chars + null
int i = 0;

MySerial.println("AT$I=10");
delay(300);

unsigned long timeout = millis() + 1000;
while (millis() < timeout && i < 8)
  {
    if (MySerial.available())
      {
        char c = MySerial.read();
        if (isxdigit(c))
          {
            idStr[i++] = c;
          }
      }
  }
idStr[8] = '\0'; // sécurité

// Convertir chaîne hex en uint32_t
uint32_t id = strtoul(idStr, NULL, 16);

// Lecture poids
float poids_g = scale.get_units(10);
if (poids_g < 0) poids_g = 0;
uint32_t poids10 = (uint32_t)(poids_g * 10);

// Créer payload (little-endian)
byte payload[8];
payload[0] = id & 0xFF;
payload[1] = (id >> 8) & 0xFF;
payload[2] = (id >> 16) & 0xFF;
payload[3] = (id >> 24) & 0xFF;
payload[4] = poids10 & 0xFF;
payload[5] = (poids10 >> 8) & 0xFF;
payload[6] = (poids10 >> 16) & 0xFF;
payload[7] = (poids10 >> 24) & 0xFF;

// Construire payload hex pour AT$SF
char hexStr[17];
sprintf(hexStr, "%02X%02X%02X%02X%02X%02X%02X%02X",
  payload[0], payload[1], payload[2], payload[3],
  payload[4], payload[5], payload[6], payload[7]);

MySerial.print("AT$SF=");
MySerial.println(hexStr);
delay(1500);

  // Coupure du capteur pour l'économie d'énergie
  digitalWrite(HX_VCC_CTRL, LOW);

  // Configuration du réveil par interruption GPIO33 (anti-vol)
  pinMode(GPIO_WAKEUP, INPUT_PULLUP); // bouton/contact = niveau bas = alerte
  esp_sleep_enable_ext0_wakeup((gpio_num_t)GPIO_WAKEUP, 0); // 0 = niveau bas déclencheur

  // Réveil automatique toutes les 3h
  //esp_sleep_enable_timer_wakeup(3ULL * 60ULL * 60ULL * 1000000ULL);  // 3 heures
  esp_sleep_enable_timer_wakeup(1800ULL * 1000000ULL);  // 30 minutes
  //esp_sleep_enable_timer_wakeup(600ULL * 1000000ULL);  // 10 minutes
  //Serial.println("\nMise en deep sleep (3h ou détection de vol)");
  esp_deep_sleep_start();
  //delay(15000);
}

void loop() {
//    MySerial.print("AT$SF=011111");
//  //Serial2.println(hexStr);
//  delay(15000);
  
  }
