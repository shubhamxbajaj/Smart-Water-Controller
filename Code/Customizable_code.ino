/* Smart Water Controller Code
 *  Written by: Shubham Bajaj
 *  Github Username: shubhamxbajaj
 *  Full documentation available on Github Repository
 */

#include <ESP8266WiFi.h> // Used to connect to a WiFi network with strongest WiFi signal
#include <WiFiClientSecure.h> // Provides Client SSL to ESP8266
#include <UniversalTelegramBot.h> // Provides an interface for using Telegram Bot API
#include "FirebaseESP8266.h" // Google Firebase Realtime Database Arduino Client Library for Espressif ESP8266

// WiFi Configuration
#define FIREBASE_HOST "YOUR FIREBASE DATABASE HOST URL"
#define FIREBASE_AUTH "YOUR FIREBASE DATABASE AUTHENTICATION KEY"
#define WIFI_SSID "YOUR WIFI SSID"
#define WIFI_PASSWORD "YOUR WIFI PASSWORD"
#define BOT_TOKEN "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

// Pin Configuration
const int TRIG_PIN = 2; // D4 PIN OF Nodemcu
const int ECHO_PIN = 0; // D3 PIN OF Nodemcu
const int WATER_PUMP_PIN = 4; // D2 PIN OF Nodemcu

// Timing and Other Constants
const int BOT_MTBS = 1000; // mean time between scan messages

// Global Variables
FirebaseData firebaseData;
float setLevelVal = 0.0;
float tankLength = 0.0;
int currentWaterLevel = 0;
bool wpOn = false;
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
long botLastTime = 0;

void setupWiFi() {
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    client.setInsecure();
}

void setupFirebase() {
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
}

// Function to read the water level using the ultrasonic sensor
int readUltrasonicSensor() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    return duration * 0.034 / 2;
}

// Function to get a float value from Firebase
float getFirebaseFloat(const String &path) {
    if (Firebase.get(firebaseData, path)) {
        return firebaseData.floatData();
    } else {
        Serial.print("Error getting ");
        Serial.print(path);
        Serial.print(": ");
        Serial.println(firebaseData.errorReason());
        return 0.0;
    }
}

// Function to set an integer value in Firebase
bool setFirebaseInt(const String &path, int value) {
    if (Firebase.setInt(firebaseData, path, value)) {
        return true;
    } else {
        Serial.print("Error setting ");
        Serial.print(path);
        Serial.print(": ");
        Serial.println(firebaseData.errorReason());
        return false;
    }
}

// Function to turn the water pump on
void turnWaterPumpOn() {
    digitalWrite(WATER_PUMP_PIN, HIGH);
    setFirebaseInt("/WP_Status", 1);
    wpOn = true;
    Serial.println("Water Pump is turned ON");
}

// Function to turn the water pump off
void turnWaterPumpOff() {
    digitalWrite(WATER_PUMP_PIN, LOW);
    setFirebaseInt("/WP_Status", 0);
    wpOn = false;
    Serial.println("Water Pump is turned OFF");
}

// Function to handle new messages from the Telegram bot
void handleTelegramMessages(int numNewMessages) {
    Serial.println("Checking For New Messages");
    Serial.println(String(numNewMessages));

    for (int i = 0; i < numNewMessages; i++) {
        String chatId = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        String fromName = bot.messages[i].from_name;

        if (fromName == "") fromName = "Guest";

        if (text == "/get_water_status") {
            String message = "The Current Water Level in your tank is " + String(currentWaterLevel) + ".\n";
            bot.sendMessage(chatId, message, "");
        } else if (text == "/Water_Pump_Status") {
            String message = "The Current Water Pump Status is " + String(wpOn ? "ON" : "OFF") + ".\n";
            bot.sendMessage(chatId, message, "");
        } else if (text == "/Tank_Percentage") {
            int tankPercentage = map(currentWaterLevel, 0, tankLength, 0, 100);
            String message = "The Current Water Percentage in your tank is " + String(tankPercentage) + "%.\n";
            bot.sendMessage(chatId, message, "");
        } else if (text == "/start") {
            String welcome = "Welcome to the Smart Water Controller Bot, " + fromName + ".\n";
            welcome += "/get_water_status : Gets the current water level in the tank.\n";
            welcome += "/Water_Pump_Status : Gets the current water pump status.\n";
            welcome += "/Tank_Percentage : Gets the percentage of water filled in the tank.\n";
            bot.sendMessage(chatId, welcome, "");
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(WATER_PUMP_PIN, OUTPUT);
    digitalWrite(WATER_PUMP_PIN, LOW);

    setupWiFi();
    setupFirebase();
}

void loop() {
    // Get tank length from Firebase
    tankLength = getFirebaseFloat("/LengthOfWaterTank");
    Serial.print("Length of water tank = ");
    Serial.println(tankLength);

    // Read water level from sensor
    currentWaterLevel = readUltrasonicSensor();
    Serial.print("Distance = ");
    Serial.println(currentWaterLevel);

    // Upload water level and percentage to Firebase
    setFirebaseInt("/waterLevel", currentWaterLevel);
    int tankPercentage = map(currentWaterLevel, 0, tankLength, 0, 100);
    setFirebaseInt("/Percent", tankPercentage);
    Serial.print("Percentage Uploaded: ");
    Serial.println(tankPercentage);

    // Control water pump based on water level
    if (currentWaterLevel < 0.2 * tankLength) {
        turnWaterPumpOn();
    } else if (currentWaterLevel > 0.95 * tankLength) {
        turnWaterPumpOff();
    }

    // Check for new messages on Telegram
    if (millis() > botLastTime + BOT_MTBS) {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages) {
            handleTelegramMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        botLastTime = millis();
    }

    // Update set water level if necessary
    setLevelVal = getFirebaseFloat("/Set_Point");
    if (setLevelVal != 0 && !wpOn) {
        while (currentWaterLevel < setLevelVal) {
            turnWaterPumpOn();
            delay(2000); // Delay to simulate time taken to fill tank
            currentWaterLevel = getFirebaseFloat("/waterLevel");
        }
        turnWaterPumpOff();
        setLevelVal = 0;
        setFirebaseInt("/Set_Point", 0);
    }

    delay(2000);
}