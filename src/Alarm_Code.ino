#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "Anass";
const char* password = "12345678";

// Telegram Bot Token and Chat ID
#define BOT_TOKEN "7265411641:AAEcLpOP9MCBUtlT7nG7otJiKIVkM0A_-GU"
#define CHAT_ID "6270611836"

// ESP32-CAM details
const String camIP = "192.168.137.62"; // Replace with your ESP32-CAM's IP
const String camPhotoPath = "/trigger_photo";

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
unsigned long lastTimeBotChecked = 0;
const unsigned long botCheckInterval = 7000; // Check every second important 

// Component Pins (on ESP32)
#define TRIG_PIN     27   // Ultrasonic TRIG
#define ECHO_PIN     36  // Ultrasonic ECHO
#define BUZZER_PIN   13
#define GREEN_LED    14
#define RED_LED      12

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Keypad config (4x4)
const byte ROWS = 4; 
const byte COLS = 4; 

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {32, 33, 25, 26};      // Outputs
byte colPins[COLS] = {4, 18, 19, 2};       // Inputs 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Security Code Logic
const String correctCode = "1234";
String enteredCode = "";
bool alarmTriggered = false;
bool photoSent = false; // To track if we've already sent a photo for this alarm

void setup() {
  Serial.begin(115200);

  // Wi-Fi Setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  client.setInsecure(); // For skipping certificate validation (optional)

  // Pin Modes
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // LCD Init
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("House Secured !");
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
}

void loop() {
  long duration;
  int distance;

  // Impulsion sur Trig
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Lire l'écho
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calcul distance
  distance = duration * 0.034 / 2;



  if (distance < 5 && !alarmTriggered) {
    triggerAlarm();
  }

  if (alarmTriggered) {
    // Keep siren beeping
    siren(BUZZER_PIN);

    // Keypad input
    char key = keypad.getKey();
    if (key) {
      enteredCode += key;
      Serial.println(enteredCode);
      lcd.setCursor(0, 1);
      lcd.print("Code: " + enteredCode + "   ");

      if (enteredCode.length() == 4) {
        if (enteredCode == correctCode) {
          Serial.println("Correct Code (Keypad)");
          resetAlarm();
        } else {
          lcd.setCursor(0, 1);
          lcd.print("Wrong code     ");
          delay(1000);
          lcd.setCursor(0, 1);
          lcd.print("Try again      ");
        }
        enteredCode = "";
      }
    }

    // Telegram input
    if (millis() - lastTimeBotChecked > botCheckInterval) {
      checkTelegram();
      lastTimeBotChecked = millis();
    }

    // Send photo request to ESP32-CAM if not already sent
    if (!photoSent) {
      triggerCameraPhoto();
      photoSent = true;
    }
  }
}

// Trigger Alarm
void triggerAlarm() {
  alarmTriggered = true;
  photoSent = false; // Reset photo sent flag
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Intruder Alert!");
  lcd.setCursor(0, 1);
  lcd.print("Enter Code");

  // Notify via Telegram
  bot.sendMessage(CHAT_ID, "🚨 Intruder Detected! Send your 4-digit code to disarm.", "");
}

// Reset Alarm
void resetAlarm() {
  alarmTriggered = false;
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER_PIN);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Granted!");
  lcd.setCursor(0, 1);
  lcd.print("House Secured !");

  bot.sendMessage(CHAT_ID, "✅ Alarm Disarmed Successfully!", "");
}

// Buzzer Siren Sound
void siren(int buzz) {
  static unsigned long lastToggleTime = 0;
  static bool isOn = false;
  unsigned long currentTime = millis();

  if (currentTime - lastToggleTime >= 300) { // toggle every 300ms
    lastToggleTime = currentTime;
    isOn = !isOn;
    if (isOn) {
      tone(buzz, 1000); // Turn buzzer on
    } else {
      noTone(buzz);     // Turn buzzer off
    }
  }
}

// Handle Telegram Code Input
void checkTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;
      String fromUser = bot.messages[i].from_name;
      String chat_id = String(bot.messages[i].chat_id);

      Serial.println("Telegram: " + fromUser + " said: " + text);

      // Only process messages from the authorized chat
      if (chat_id == CHAT_ID) {
        if (text == correctCode) {
          resetAlarm();
        } else if (text.length() == 4) { // Only check 4-digit codes
          bot.sendMessage(CHAT_ID, "❌ Wrong code. Try again.", "");
        }
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// Trigger ESP32-CAM to take and send photo
void triggerCameraPhoto() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String serverPath = "http://" + camIP + camPhotoPath;
    
    http.begin(serverPath.c_str());
    
    // Send HTTP GET request
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      
      // Notify user that photo is being sent
      bot.sendMessage(CHAT_ID, "📸 Taking photo of intruder...", "");
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      bot.sendMessage(CHAT_ID, "⚠️ Failed to trigger camera. Check camera connection.", "");
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}