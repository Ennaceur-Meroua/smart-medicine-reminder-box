#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <ESP32Servo.h>
#include <HardwareSerial.h>
#include <RTClib.h>

// --- CONFIGURATION ---
HardwareSerial SIM800L(2); 
const char* numMalade  = "xxxxxxxxxxxxx"; 
const char* numFamille = "xxxxxxxxxxxxx"; 
#define RST_PIN   27   
#define SS_PIN    5   
const int buzzerPin    = 13; 
const int boutonPin    = 12;
const int ledVertePin  = 32;
const int ledOrangePin = 25; 
const int ledRougePin  = 26; 
const int servoPin     = 2;

// --- OBJETS & VARIABLES ---
RTC_DS3231 rtc;
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_PCF8574 lcd(0x27); 
Servo monServo;

int stockAmlodipine = 4; 
const int dose = 2;
int phaseDemo = 1; 
DateTime heureSauvegardee;
unsigned long millisSauvegarde;
bool enModeTest = false;
bool forcageAllumage = false; 
bool alerteEnCours = false; 
unsigned long timeoutEcran = 0; 

byte icons[4][8] = {
  {B00100,B00100,B11111,B00100,B00100,0,0,0}, 
  {B01110,B10001,B10001,B01110,B01110,B10001,B10001,B01110}, 
  {B01010,B11111,B11111,B01110,B00100,0,0,0}, 
  {B01110,B10001,B10101,B10101,B10001,B01110,0,0}  
};

// --- FONCTIONS ---
void rafraichirAffichageHeure() {
  if (alerteEnCours) return; 
  DateTime now = rtc.now();
  lcd.setCursor(4, 0);
  if(now.hour() < 10) lcd.print('0'); lcd.print(now.hour()); lcd.print(':');
  if(now.minute() < 10) lcd.print('0'); lcd.print(now.minute()); lcd.print(':');
  if(now.second() < 10) lcd.print('0'); lcd.print(now.second());
  lcd.setCursor(3, 1);
  if(now.day() < 10) lcd.print('0'); lcd.print(now.day()); lcd.print('/');
  if(now.month() < 10) lcd.print('0'); lcd.print(now.month()); lcd.print('/');
  lcd.print(now.year());
}

// Fonction SMS nettoyée pour garantir la transmission
void envoyerSMS(const char* numero, String contenu) {
  SIM800L.println("AT+CMGF=1"); 
  delay(500);
  SIM800L.print("AT+CMGS=\""); SIM800L.print(numero); SIM800L.println("\"");
  delay(500);
  SIM800L.print(contenu); 
  delay(500);
  SIM800L.write(26); 
  delay(5000); 
}



void bougerServo(int angle) {
  monServo.attach(servoPin);
  delay(50); monServo.write(angle); delay(1000); 
}

void bipManuel(int duree) {
  digitalWrite(buzzerPin, HIGH); delay(duree); digitalWrite(buzzerPin, LOW);
}

void gestionCasVide() {
  alerteEnCours = true;
  digitalWrite(ledOrangePin, HIGH);
  lcd.setBacklight(255);
  String msg = "Remplir le compartiment 1 avec Amlodipine        ";
  unsigned long startR = millis();
  while (millis() - startR < 10000) {
    lcd.setCursor(0, 0); lcd.print("   Stock vide   ");
    for (int i = 0; i < msg.length() - 15; i++) {
      lcd.setCursor(0, 1); lcd.print(msg.substring(i, i + 16));
      delay(250); if (millis() - startR >= 10000) break;
    }
  }
  stockAmlodipine = 4;
  digitalWrite(ledOrangePin, LOW);
  for(int k=0; k<3; k++) { bipManuel(150); delay(150); }
  lcd.clear(); lcd.setCursor(0, 0); lcd.print("Stock mis a jour");
  lcd.setCursor(0, 1); lcd.print("Stock actuel: 4"); 
  delay(3000);
  alerteEnCours = false;
}

void executerScenario() {
  alerteEnCours = true;
  lcd.setBacklight(255);
  lcd.clear(); lcd.setCursor(3, 0); lcd.print("Bienvenue"); delay(1000);
  
  lcd.clear(); lcd.setCursor(0, 0); lcd.print("Pilulier Intell.");
  unsigned long startAnim = millis();
  while(millis() - startAnim < 3000) {
    lcd.setCursor(4, 1); for(int i=0; i<4; i++) { lcd.write(i); lcd.print(" "); }
    delay(400); lcd.setCursor(4, 1); lcd.print("               "); delay(400);
  }

  digitalWrite(ledVertePin, HIGH);

  // === OUVERTURE : Le bras tourne à 85° ===
  bougerServo(85); 

  bool tagValide = false;
  while (!tagValide) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      String content = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
         content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
         content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      content.toUpperCase();
      if (content == "A51CD205") tagValide = true;
      
    }
    // Afficher "Tag non valide" si l'ID du Tag lu, n'est pas la bonne
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Tag non valide");
    delay(500);
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Recommancez");
    delay(500);
  }
  // Afficher "Tag valide" si l'ID du tag lu est correcte
  lcd.clear(); lcd.setCursor(0, 0); lcd.print("Tag valide");
  delay(2000);

  lcd.clear(); lcd.setCursor(0, 0); lcd.print("Compartiment 1");
  lcd.setCursor(0, 1); lcd.print("Amlodipine"); delay(3000);
  stockAmlodipine -= dose;
  lcd.clear(); lcd.print("A prendre...");
  for (int i = 16; i >= 0; i--) {
    lcd.setCursor(0, 1);
    for (int j = 0; j < 16; j++) { if (j < i) lcd.print((char)255); else lcd.print(" "); }
    delay(625); 
  }
  
    if (stockAmlodipine <= 0) gestionCasVide();

    // Ouverture  du Pillulier pour recharger le stock
  bougerServo(85); digitalWrite(ledVertePin, LOW); lcd.clear();
      alerteEnCours = false;

    // Fermeture du Pillulier apres la prise de medicament
  bougerServo(5);

}

void lancerCycleAlerte() {
  alerteEnCours = true;
  lcd.setBacklight(255);
  lcd.clear(); lcd.setCursor(0, 0); lcd.print(" HEURE DE PRISE ");
  lcd.setCursor(1, 1); lcd.print("Appuyez bouton");
  unsigned long debut = millis();
  bool patientPresent = false;
  while (millis() - debut < 20000) {
    digitalWrite(buzzerPin, HIGH); delay(150); digitalWrite(buzzerPin, LOW); 
    if (digitalRead(boutonPin) == LOW) { patientPresent = true; break; }
    delay(350);
  }
  if (patientPresent) {
    bipManuel(100); while(digitalRead(boutonPin) == LOW); 
    executerScenario();    
  } else {
    digitalWrite(ledRougePin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(" Prise oubliee! "); 
    lcd.setCursor(1, 1); lcd.print("  SMS envoye !  "); 
    envoyerSMS(numMalade, "[Pilulier intelligent]: oublie detecte.Vous n'avez pas pris votre medicament.Veuillez le prendre maintenant.");
    envoyerSMS(numFamille, "[Pilulier intelligent]:Notification d'anomalie.La prise du traitement programmee n'a pas ete validee.Une intervention ou verification est recommandee.");
    delay(10000); 
    digitalWrite(ledRougePin, LOW);
    lcd.clear();
  }
  alerteEnCours = false;
}

void setup() {
  pinMode(buzzerPin, OUTPUT); pinMode(ledVertePin, OUTPUT);
  pinMode(ledOrangePin, OUTPUT); pinMode(ledRougePin, OUTPUT);
  pinMode(boutonPin, INPUT_PULLUP);
  Wire.begin(21, 22); lcd.begin(16, 2); lcd.setBacklight(0); 
  for(int i=0; i<4; i++) lcd.createChar(i, icons[i]);
  SPI.begin(18, 19, 23, 5); mfrc522.PCD_Init();
  SIM800L.begin(9600, SERIAL_8N1, 16, 17);
  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   
   
   // === FERMETURE : Le bras revient à sa position  ===
  bougerServo(5); 
  monServo.detach(); 
}

void loop() {
  DateTime now = rtc.now();

  if (forcageAllumage || millis() < timeoutEcran) {
    lcd.setBacklight(255);
    rafraichirAffichageHeure();
  } else {
    if (!alerteEnCours) { lcd.setBacklight(0); lcd.clear(); }
  }

  if (now.hour() == 19 && now.minute() == 59 && now.second() == 20) {
    envoyerSMS(numMalade, "[Pilulier intelligent]: Alerte preventive- stock faible (2 comprimes d'amlodipine).Pensez a remplir le compartiment 1 lors de la prochaine prise.");
  }
  if ((now.hour() == 8 || now.hour() == 20) && now.minute() == 0 && now.second() == 0) {
    lancerCycleAlerte();
  }

  if (digitalRead(boutonPin) == LOW) {
    
    unsigned long press = millis();
    while (digitalRead(boutonPin) == LOW) {
      if (millis() - press >= 3000) { 
        bipManuel(200);
              if (!enModeTest) { 
          heureSauvegardee = rtc.now(); millisSauvegarde = millis();
          enModeTest = true; forcageAllumage = true; 
         rtc.adjust(DateTime(now.year(), now.month(), now.day(), 7, 59, 50));
          phaseDemo = 2;
          lcd.clear(); 
        } 
        else if (enModeTest && phaseDemo == 2) { 
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), 19, 59, 15));
          phaseDemo = 0;
          lcd.clear();
        }
        else if (enModeTest && phaseDemo == 0) { 
          unsigned long ecartSec = (millis() - millisSauvegarde) / 1000;
          rtc.adjust(DateTime(heureSauvegardee.unixtime() + ecartSec));
          enModeTest = false; forcageAllumage = false; phaseDemo = 1;
          bipManuel(500); 
          timeoutEcran = millis() + 7000;
          lcd.clear(); // Efface les anciens caractères bizarres
          rafraichirAffichageHeure(); // Affiche immédiatement l'heure réelle propre
        }
        while(digitalRead(boutonPin) == LOW); 
        return;
      }
    }
    if (!enModeTest) timeoutEcran = millis() + 30000; 
  }
  delay(50);
}

