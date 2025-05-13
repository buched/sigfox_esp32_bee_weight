#include "HX711.h"

#define HX_DT 19
#define HX_SCK 18
#define HX_VCC_CTRL 23

HX711 scale;

void setup() {
  Serial.begin(115200);
  // Alimentation du HX711 via GPIO
  pinMode(HX_VCC_CTRL, OUTPUT);
  digitalWrite(HX_VCC_CTRL, HIGH);
  delay(100);  // Laisser le capteur démarrer
  
  scale.begin(HX_DT, HX_SCK);
  Serial.println("Retirer tout poids du capteur...");
  delay(1000);
Serial.println("3...");
  delay(1000);
  Serial.println("2...");
  delay(1000);
  Serial.println("1...");
  delay(1000);
  scale.tare();  // mise à zéro
  Serial.println("Tare terminée. Place un poids connu...");
  delay(1000);
  Serial.println("5...");
  delay(1000);
  Serial.println("4...");
  delay(1000);
  Serial.println("3...");
  delay(1000);
  Serial.println("2...");
  delay(1000);
  Serial.println("1...");
  delay(1000);
  long raw = scale.get_value(10);  // moyenne brute sur 10 lectures
  Serial.print("Valeur brute : ");
  Serial.println(raw);

  float poids_reel = 923.0;  // ← remplace par la masse exacte (en grammes)
  float facteur = raw / poids_reel;

  Serial.print("Facteur d'échelle calculé : ");
  Serial.println(facteur, 3);
}

void loop() {}
