#include "HX711.h"
#include <HardwareSerial.h>

HardwareSerial MySerial(1);

// HX711
#define HX_DT 10
#define HX_SCK 2
#define HX_VCC_CTRL 3

// Capteur anti-vol
//#define GPIO_WAKEUP 1

HX711 scale;

// Variable conservée pendant le deep sleep uniquement
RTC_DATA_ATTR bool dejaInitialise = false;

bool envoyerSigfoxAvecRetries(String hexPayload) {
  const int MAX_RETRIES = 3;
  const unsigned long SIGFOX_TIMEOUT = 10000; // 10 sec
  const unsigned long RETRY_DELAY = 10000;    // 10 sec

  for (int tentative = 1; tentative <= MAX_RETRIES; tentative++) {
    Serial.print("Tentative ");
    Serial.println(tentative);

    MySerial.print("AT$SF=");
    MySerial.print(hexPayload);
    MySerial.print("\r");

    // Attente d'un OK
    unsigned long t0 = millis();
    while (millis() - t0 < SIGFOX_TIMEOUT) {
      if (MySerial.available()) {
        String ligne = MySerial.readStringUntil('\n');
        ligne.trim();
        Serial.println(">> " + ligne);
        if (ligne == "OK") {
          Serial.println("Message SigFox envoyé avec succès !");
          return true;
        } else if (ligne.startsWith("ERROR")) {
          break;
        }
      }
    }

    Serial.println("Échec de l'envoi. Nouvelle tentative dans 10 sec...");
    delay(RETRY_DELAY);
  }

  Serial.println("Échec définitif après 3 tentatives.");
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(1000); // stabilité
  MySerial.begin(9600, SERIAL_8N1, 21, 20);
  delay(1000); // attendre que le module SigFox soit prêt

  // Alimentation HX711
  pinMode(HX_VCC_CTRL, OUTPUT);
  digitalWrite(HX_VCC_CTRL, HIGH);
  delay(1000);

  // Init HX711
  scale.begin(HX_DT, HX_SCK);
  scale.set_scale(48.182);
    // Faire la tare uniquement au tout premier démarrage (alimentation), Nb, normalement décommenté mais certains esp chinois ne supportent pas cette fonction
  if (!dejaInitialise) 
    {
      scale.tare();
    }
  delay(100);

  // Lecture poids
  float poids_g = scale.get_units(10);
  if (poids_g < 0) poids_g = 0;
  uint32_t poids10 = (uint32_t)(poids_g * 10);
  Serial.print("Poids (x10g) : ");
  Serial.println(poids10);

  // Payload (4 octets, little-endian)
  byte payload[4];
  payload[0] = poids10 & 0xFF;
  payload[1] = (poids10 >> 8) & 0xFF;
  payload[2] = (poids10 >> 16) & 0xFF;
  payload[3] = (poids10 >> 24) & 0xFF;

  // Hex string
  char hexStr[9];
  sprintf(hexStr, "%02X%02X%02X%02X", payload[0], payload[1], payload[2], payload[3]);
  String hexPayload = String(hexStr);
  Serial.print("Payload Hex : ");
  Serial.println(hexPayload);

  // Envoi avec 3 tentatives
  bool success = envoyerSigfoxAvecRetries(hexPayload);

  // Extinction HX711
  digitalWrite(HX_VCC_CTRL, LOW);

  // Wakeup, decommenter la ligen désirée et commenter les 2 autres
  //esp_sleep_enable_timer_wakeup(600ULL * 1000000ULL);  // 10 minutes
  esp_sleep_enable_timer_wakeup(1800ULL * 1000000ULL);  // 30 minutes
  //esp_sleep_enable_timer_wakeup(3ULL * 60ULL * 60ULL * 1000000ULL);  // 3 heures
  Serial.println("Mise en veille profonde...");
  delay(100);
  esp_deep_sleep_start();
}

void loop() {}
