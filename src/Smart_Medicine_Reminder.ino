#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <ESP32Servo.h>
#include <HardwareSerial.h>
#include <RTClib.h>

// --- CONFIGURATION ---
HardwareSerial SIM800L(2);
const char* patientNumber = "xxxxxxxxxxxxx";
const char* familyNumber = "xxxxxxxxxxxxx";

#define RST_PIN 27
#define SS_PIN 5

const int buzzerPin = 13;
const int buttonPin = 12;
const int greenLedPin = 32;
const int orangeLedPin = 25;
const int redLedPin = 26;
const int servoPin = 2;

// --- OBJECTS & VARIABLES ---
RTC_DS3231 rtc;
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_PCF8574 lcd(0x27);
Servo servoMotor;

int amlodipineStock = 4;
const int dose = 2;
int demoPhase = 1;
DateTime savedTime;
unsigned long savedMillis;
bool testMode = false;
bool forceDisplayOn = false;
bool alertInProgress = false;
unsigned long displayTimeout = 0;

byte icons[4][8] = {
  {B00100,B00100,B11111,B00100,B00100,0,0,0},
  {B01110,B10001,B10001,B01110,B01110,B10001,B10001,B01110},
  {B01010,B11111,B11111,B01110,B00100,0,0,0},
  {B01110,B10001,B10101,B10101,B10001,B01110,0,0}
};

// --- FUNCTIONS ---
void refreshTimeDisplay() {
  if (alertInProgress) return;
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

// Send SMS notification
void sendSMS(const char* number, String message) {
  SIM800L.println("AT+CMGF=1");
  delay(500);
  SIM800L.print("AT+CMGS=\""); SIM800L.print(number); SIM800L.println("\"");
  delay(500);
  SIM800L.print(message);
  delay(500);
  SIM800L.write(26);
  delay(5000);
}

void moveServo(int angle) {
  servoMotor.attach(servoPin);
  delay(50); servoMotor.write(angle); delay(1000);
}

void manualBeep(int duration) {
  digitalWrite(buzzerPin, HIGH); delay(duration); digitalWrite(buzzerPin, LOW);
}

void handleEmptyStock() {
  alertInProgress = true;
  digitalWrite(orangeLedPin, HIGH);
  lcd.setBacklight(255);

  String message = "Refill compartment 1 with Amlodipine        ";
  unsigned long startTime = millis();

  while (millis() - startTime < 10000) {
    lcd.setCursor(0, 0); lcd.print("   Stock empty   ");

    for (int i = 0; i < message.length() - 15; i++) {
      lcd.setCursor(0, 1); lcd.print(message.substring(i, i + 16));
      delay(250);
      if (millis() - startTime >= 10000) break;
    }
  }

  amlodipineStock = 4;
  digitalWrite(orangeLedPin, LOW);

  for(int k = 0; k < 3; k++) {
    manualBeep(150);
    delay(150);
  }

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Stock updated");
  lcd.setCursor(0, 1); lcd.print("Current stock: 4");
  delay(3000);

  alertInProgress = false;
}

void runMedicationScenario() {
  alertInProgress = true;
  lcd.setBacklight(255);
  lcd.clear();
  lcd.setCursor(3, 0); lcd.print("Welcome");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Smart Pill Box");

  unsigned long animationStart = millis();
  while(millis() - animationStart < 3000) {
    lcd.setCursor(4, 1);

    for(int i = 0; i < 4; i++) {
      lcd.write(i);
      lcd.print(" ");
    }

    delay(400);
    lcd.setCursor(4, 1);
    lcd.print("               ");
    delay(400);
  }

  digitalWrite(greenLedPin, HIGH);

  // Open the medication box
  moveServo(85);

  bool validTag = false;

  while (!validTag) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      String content = "";

      for (byte i = 0; i < rfid.uid.size; i++) {
        content.concat(String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""));
        content.concat(String(rfid.uid.uidByte[i], HEX));
      }

      content.toUpperCase();
      if (content == "A51CD205") validTag = true;
    }

    // Display message if the RFID tag is invalid
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Invalid tag");
    delay(500);

    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Try again");
    delay(500);
  }

  // Display message if the RFID tag is valid
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Valid tag");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Compartment 1");
  lcd.setCursor(0, 1); lcd.print("Amlodipine");
  delay(3000);

  amlodipineStock -= dose;

  lcd.clear();
  lcd.print("Take medication");

  for (int i = 16; i >= 0; i--) {
    lcd.setCursor(0, 1);
    for (int j = 0; j < 16; j++) {
      if (j < i) lcd.print((char)255);
      else lcd.print(" ");
    }

    delay(625);
  }

  if (amlodipineStock <= 0) handleEmptyStock();

  // Open the box for stock refill
  moveServo(85);
  digitalWrite(greenLedPin, LOW);
  lcd.clear();

  alertInProgress = false;

  // Close the medication box
  moveServo(5);
}

void startMedicationAlert() {
  alertInProgress = true;
  lcd.setBacklight(255);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(" MEDICATION TIME ");
  lcd.setCursor(1, 1); lcd.print("Press button");

  unsigned long startTime = millis();
  bool patientPresent = false;

  while (millis() - startTime < 20000) {
    digitalWrite(buzzerPin, HIGH);
    delay(150);
    digitalWrite(buzzerPin, LOW);

    if (digitalRead(buttonPin) == LOW) {
      patientPresent = true;
      break;
    }

    delay(350);
  }

  if (patientPresent) {
    manualBeep(100);
    while(digitalRead(buttonPin) == LOW);
    runMedicationScenario();
  } else {
    digitalWrite(redLedPin, HIGH);

    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Medication missed");
    lcd.setCursor(1, 1); lcd.print("  SMS sent!  ");

    sendSMS(patientNumber,
      "[Smart Pill Box]: Missed medication detected. Please take your medication now.");

    sendSMS(familyNumber,
      "[Smart Pill Box]: Scheduled medication intake was not validated. Verification is recommended.");

    delay(10000);

    digitalWrite(redLedPin, LOW);
    lcd.clear();
  }

  alertInProgress = false;
}

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(orangeLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  Wire.begin(21, 22);
  lcd.begin(16, 2);
  lcd.setBacklight(0);

  for(int i = 0; i < 4; i++) lcd.createChar(i, icons[i]);

  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();

  SIM800L.begin(9600, SERIAL_8N1, 16, 17);
  rtc.begin();

  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Close the medication box at startup
  moveServo(5);
  servoMotor.detach();
}

void loop() {
  DateTime now = rtc.now();

  if (forceDisplayOn || millis() < displayTimeout) {
    lcd.setBacklight(255);
    refreshTimeDisplay();
  } else {
    if (!alertInProgress) {
      lcd.setBacklight(0);
      lcd.clear();
    }
  }

  // Low-stock warning
  if (now.hour() == 19 && now.minute() == 59 && now.second() == 20) {
    sendSMS(patientNumber,
      "[Smart Pill Box]: Low stock warning - 2 Amlodipine tablets remain. Please refill compartment 1.");
  }

  // Medication schedule: 08:00 and 20:00
  if ((now.hour() == 8 || now.hour() == 20) &&
      now.minute() == 0 && now.second() == 0) {
    startMedicationAlert();
  }

  if (digitalRead(buttonPin) == LOW) {
    unsigned long pressStart = millis();

    while (digitalRead(buttonPin) == LOW) {
      if (millis() - pressStart >= 3000) {
        manualBeep(200);

        if (!testMode) {
          savedTime = rtc.now();
          savedMillis = millis();

          testMode = true;
          forceDisplayOn = true;

          rtc.adjust(DateTime(now.year(), now.month(), now.day(), 7, 59, 50));
          demoPhase = 2;
          lcd.clear();
        }
        else if (testMode && demoPhase == 2) {
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), 19, 59, 15));
          demoPhase = 0;
          lcd.clear();
        }
        else if (testMode && demoPhase == 0) {
          unsigned long elapsedSeconds = (millis() - savedMillis) / 1000;

          rtc.adjust(DateTime(savedTime.unixtime() + elapsedSeconds));

          testMode = false;
          forceDisplayOn = false;
          demoPhase = 1;

          manualBeep(500);
          displayTimeout = millis() + 7000;

          lcd.clear();
          refreshTimeDisplay();
        }

        while(digitalRead(buttonPin) == LOW);
        return;
      }
    }

    if (!testMode) displayTimeout = millis() + 30000;
  }

  delay(50);
}
