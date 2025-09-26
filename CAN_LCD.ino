/*

VCC → 5V
GND → GND
SDA → A4
SCL → A5 
LED → 5V



CS: Pin 10
SCK: Pin 13
MOSI: Pin 11
MISO: Pin 12
VCC: 5V
GND: GND
INT : 2

*/

/*

affiche soc assad avec connexion avec high low de  l'ecran
rxBuf[i] = 


*/

//18FE28F4               0x18FF28F4


#include <mcp_can.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];
String msg;

// ID de la requête CAN à envoyer au BMS (à adapter selon le protocole du BMS)
#define REQUEST_ID 0x18FF28F4 // ID de la requête (vérifiez dans la documentation du BMS)
#define RESPONSE_ID 0x18FF28F4 // ID de la réponse attendue
/*
#define REQUEST_ID 0x18100140     // PC send (selon doc)
#define RESPONSE_ID 0x18104001    // BMS response (selon doc)
*/

#define CAN0_INT 2  // Pin d'interruption
MCP_CAN CAN0(10);   // Pin CS pour MCP2515

// Initialisation de l'écran LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adresse 0x27, 16 colonnes, 2 lignes

// Variables pour gérer l'envoi périodique de la requête
unsigned long lastRequestTime = 0;
const unsigned long requestInterval = 2000; // Intervalle de 2 secondes pour envoyer la requête


// Fonction pour envoyer une requête CAN au BMS
void sendCanRequest() {
  unsigned char data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Données de la requête (à adapter)
  byte sndStat = CAN0.sendMsgBuf(REQUEST_ID, 1, 8, data); // ID, Extended Frame (1), longueur, données

  if (sndStat == CAN_OK) {
    Serial.println("Requête CAN envoyée avec succès");
  } else {
    Serial.println("Erreur lors de l'envoi de la requête CAN");
  }
}


void setup() {
  Serial.begin(115200);
  
  // Initialisation de l'écran LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initialisation...");

  // Initialisation du MCP2515

  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully!");
    lcd.clear();
    lcd.print("CAN OK");
  } else {
    Serial.println("Error Initializing MCP2515...");
    lcd.clear();
    lcd.print("CAN Error");
  while (1); // Arrêt en cas d'erreur
  }


  CAN0.setMode(MCP_NORMAL); // Mode normal pour envoyer/recevoir des messages
  pinMode(CAN0_INT, INPUT); // Configuration de la broche d'interruption
  Serial.println("MCP2515 Library Receive SoC...");
}

void loop() {
  // Envoyer une requête CAN au BMS toutes les 2 secondes
  unsigned long currentTime = millis();
  if (currentTime - lastRequestTime >= requestInterval) {
    sendCanRequest();
    lastRequestTime = currentTime;
  }

  // Lire les messages CAN si une interruption est détectée
  if (!digitalRead(CAN0_INT)) {
    CAN0.readMsgBuf(&rxId, &len, rxBuf); // Lire le message CAN

    // Filtrer les messages avec l'ID attendu
    if ((rxId & 0x1FFFFFFF) == RESPONSE_ID) {
      if ((rxId & 0x40000000) == 0x40000000) {
        // Ignorer les trames de requête distante
      } else {
        // Afficher le SoC (2ème byte) sur le port série et l'écran LCD
        Serial.print("SoC = ");
        Serial.print(rxBuf[1]);
        Serial.println("%");
        

        lcd.clear();
        lcd.setCursor(0, 0);
        String socString = "SoC = " + String(rxBuf[1]) + "%";
        lcd.print(socString);
      }
    }
  }
}

