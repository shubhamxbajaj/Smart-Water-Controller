/* Smart Water Controller Code
 *  Written by: Shubham Bajaj
 *  Github Username: shubhamxbajaj
 *  Full documentation available on Github Repository
 */



#include <ESP8266WiFi.h> // Used to connect to a WiFi network with strongest WiFi signal
#include <WiFiClientSecure.h> // Provides Client SSL to ESP8266
#include <UniversalTelegramBot.h> // Provides an interface for using Telegram Bot API
#include "FirebaseESP8266.h" // Google Firebase Realtime Database Arduino Client Library for Espressif ESP8266

#define FIREBASE_HOST "YOUR FIREBASE DATABASE HOST URL"  // Enter Your credentials for initialization
#define FIREBASE_AUTH "YOUR FIREBASE DATABASE AUTHENTICATION KEY" // Enter Your credentials for initialization
#define WIFI_SSID "YOUR WIFI SSID" // Enter Your credentials for initialization
#define WIFI_PASSWORD "YOUR WIFI PASSWORD" // Enter Your credentials for initialization
#define BOTtoken "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  // Telegram BOT Token (Get from Botfather)

const int trigP = 2;  //D4 PIN OF Nodemcu
const int echoP = 0;  // D3 PIN OF Nodemcu


//1. Define FirebaseESP8266 data object for sending and receiving data
FirebaseData firebaseData; //Define the Firebase Data object
float SetLevelVal = 0; //Define the SetLevelVal Variable.
int water_pump = 4; // D2 PIN OF Nodemcu(For Waterpump relay)
boolean WP_ON=false; //Define the WP_ON Variable
float Tanklevel = 0; //Define the Tanklevel Variable
float Length = 0; //Define the Length Variable
long duration; //Define the duration Variable
int current_water_level; //Define the current_water_level Variable
WiFiClientSecure client; //Create TLS connection 
UniversalTelegramBot bot(BOTtoken, client); // Initialization for Telegram bot
int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;  //Define the Start Variable



void setup() {
  Serial.begin(115200); //Opens serial port, sets data rate to 115200 bps 
    pinMode(water_pump,OUTPUT); // Setting the Water Pumb as a Output
      digitalWrite(water_pump,LOW); // Water Pumb set to Off 
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Connects to WIFI Network
          Serial.print("Connecting to Wi-Fi");
            while (WiFi.status() != WL_CONNECTED)
              {
                  Serial.print(".");
                      delay(300);
                        }
                          Serial.println();
                            Serial.print("Connected with IP: ");
                              Serial.println(WiFi.localIP()); // Print Local IP Address
                                Serial.println();
                                client.setInsecure(); // Used to make https requests


  //2. Set your Firebase info
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); //Begins connection with Firebase Database
  //3. Enable auto reconnect the WiFi when connection lost
    Firebase.reconnectWiFi(true);

pinMode(trigP, OUTPUT);  // Sets the trigPin as an Output
pinMode(echoP, INPUT);  // Sets the echoPin as an Input
}


void loop() {
  // These commands get the Length of water tank by loading it from the firebase database
   if(Firebase.get(firebaseData, "/LengthOfWaterTank")) // LengthOfWaterTank is a variable name in my Firebase database. Change it accordingly
     {
           Length = firebaseData.intData(); // Updates Length variable locally
                 Serial.print("Length of water tank = "); 
                       Serial.println(Length); // Print Length 
                         }
                           else
                             {
                                 Serial.print("Error in get, ");
                                     Serial.println(firebaseData.errorReason()); // If error in loading the value then prints reason for it 
                             }
    // Calculation of Distance Using Ultrasonic Sensor(HC-SR04)
    digitalWrite(trigP, LOW);   // Makes trigPin low
    delayMicroseconds(2);       // 2 micro second delay 
    digitalWrite(trigP, HIGH);  // tigPin high
    delayMicroseconds(10);      // trigPin high for 10 micro seconds
    digitalWrite(trigP, LOW);   // trigPin low
    duration = pulseIn(echoP, HIGH);   //Read echo pin, time in microseconds
    current_water_level= duration*0.034/2;        //Calculating actual/real current_water_level
    Serial.print("Distance = ");        //Output current_water_level on arduino serial monitor 
    Serial.println(current_water_level);           //Prints current_water_level
    delay(3000);
    // These commands upload the current water level on the firebase database
    if(Firebase.setInt(firebaseData, "/waterLevel",current_water_level)) // waterLevel is a variable name in my Firebase database. Change it accordingly
    { 
      }
      else 
      {
        Serial.print("Error in get, ");
        Serial.println(firebaseData.errorReason()); // If error in uploading the current water level then prints reason for it 
        }

    Tanklevel = map(current_water_level,0,Length,0,100); // Changing range to percentage
    // These commands upload the percentage of water tank filed on the firebase database
    if(Firebase.setInt(firebaseData, "/Percent", Tanklevel)) // Percent is a variable name in my Firebase database. Change it accordingly
      {
           Serial.println("Percentage Uploaded:");
           Serial.println(Tanklevel); // Prints percentage
             }
               else
                 {
                     Serial.print("Error in setInt, ");
                         Serial.println(firebaseData.errorReason()); // If error in uploading the percentage of water tank filed then prints reason for it
                           }
    // These commands check if the current water level is less than 20% of the total tank then automatically turn on the water pumb to fill water.
    if(current_water_level<(0.2*Length)) // if Water level is less than 20% turn on the Water Pumb
    {
      digitalWrite(water_pump,HIGH); // turn on Water Pumb 
          //Send water pump status to firebase/
      if(Firebase.setInt(firebaseData, "/WP_Status", 1))  // Sets WP_Status to 1 or ON
        {
             Serial.println("Water Pump is turned ON"); //Print Water pump status
               }
                 else
                   {
                       Serial.print("Error in setInt, ");
                           Serial.println(firebaseData.errorReason()); // If error in uploading the water pumb status then prints reason for it 
                             }
                               }
    
    // These commands check if the current water level is greater than 95% of the total tank then automatically turn off the water pumb.
    if(current_water_level>(0.95*Length)) //if Water Level is grater than 95%
    {
       digitalWrite(water_pump,LOW); // turn off water pumb 
           //Send water pump status to firebase/
      if(Firebase.setInt(firebaseData, "/WP_Status", 0))   // Sets WP_Status Value in database to 0 or OFF
        {
             Serial.println("Water Pump is turned OFF"); //Prints water pump status
               }
                 else
                   {
                       Serial.print("Error in setInt, ");
                           Serial.println(firebaseData.errorReason()); //If error in uploading the water pumb status then prints reason for it 
                             }
        }
              delay(2000);
    // These commands are used to check if any new message is received on telegram.                        
    if (millis() > Bot_lasttime + Bot_mtbs)  {       //Checks for new messages on telegram
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while (numNewMessages) {                       // If a new message is received on telegram 
        Serial.println("got response");
        handleNewMessages(numNewMessages);           // Message sent to handleNewMessages function declared below
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
  
      Bot_lasttime = millis();
    }
    // These commands get the Set water level value by loading it from the firebase database
    if(Firebase.get(firebaseData, "/Set_Point"))  // Set_Point is a variable name in my Firebase database. Change it accordingly
    {
             SetLevelVal = firebaseData.intData();  // Updates SetLevelVal variable 
             }
             else
             {
              Serial.print("Error in get, ");
              Serial.println(firebaseData.errorReason()); // If error in loading the Set water level value then prints reason for it 
              }
   
    // These commands check if given water Level is less than Set water level value and if water pumb is turned off
    if(SetLevelVal != 0 && WP_ON==false)   // Checking condition
    {
      while(current_water_level != SetLevelVal)   // if water level is less then set level
        { 
            WP_ON=true; // Updating WP_ON variable
            digitalWrite(water_pump,LOW); // Turning on Water Pumb  
             if(Firebase.get(firebaseData, "/waterLevel")) // waterLevel is a variable name in my Firebase database. Change it accordingly
             {
              current_water_level = firebaseData.intData(); //Updating current water level variable in while loop
              }
            if(Firebase.setInt(firebaseData, "/WP_Status", 1)) // Sets WP_Status to 1 or ON
            {
               Serial.print("SetLevel = ");
               Serial.println(SetLevelVal); //This prints the set water level value
                 }
            Tanklevel = map(current_water_level,0,Length,0,100); // Changing range to percentage in while loop
            if(Firebase.setInt(firebaseData, "/Percent", Tanklevel)) // Updating Percent variable in while loop
            {
           Serial.println("Percentage Uploaded");
           Serial.println(Tanklevel); // Prints percentage of water tank filled
             }                          
          }
      WP_ON=false; // Updating WP_ON variable after successfully reaching the desired water level
      SetLevelVal=0; // Setting SetLevelVal variable to 0 after successfully reaching the desired water level
      if(Firebase.setInt(firebaseData, "/Set_Point", 0)) // Setting Set_Point variable to 0 after successfully reaching the desired water level
          {
            }
      digitalWrite(water_pump,LOW); // turn off water pumb 
      if(Firebase.setInt(firebaseData, "/WP_Status", 0)) // WP_Status is a variable name in my Firebase database. Change it accordingly
          {
            }
            else
            {
              Serial.print("Error in setInt, ");
              Serial.println(firebaseData.errorReason()); //If error in uploading the water pumb status then prints reason for it 
              } 
       }
                    //Send water pump status to firebase/
       
  }


// This section includes Telegram Bot code
void handleNewMessages(int numNewMessages) {
    Serial.println("Checking For New Messages");
    Serial.println(String(numNewMessages));
  
  
    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;
  
      String from_name = bot.messages[i].from_name;
      if (from_name == "") from_name = "Guest";
  
      if (text == "/get_water_status") {
        String Wlevel = "The Current Water Level In your water tank is, " + String(current_water_level)+ ".\n" ;
        bot.sendMessage(chat_id, Wlevel, "");
      }
      
      if (text == "/Water_Pump_Status") { 
        if (WP_ON == 0) {
          bot.sendMessage(chat_id, "The Current Water Pump Status is, OFF",".\n");
        }
        else{
          bot.sendMessage(chat_id, "The Current Water Pump Status is, ON",".\n");
        }
        
      }
  
      if (text == "/Tank_Percentage") {
        String tlevel = "The Current Water Percentage in your tank is, " + String(Tanklevel) + ".\n";
        bot.sendMessage(chat_id, tlevel, "");
      }


      // All the messages can be customised 
      if (text == "/start") {
        String welcome = "Welcome to Telegram Bot, " + from_name + ".\n";
        welcome += "This will show several properies.\n\n";
        welcome += "/get_water_status : Gets The Current Water Level In Tank\n";
        welcome += "/Water_Pump_Status : Gets The Current Water Pump Status \n";
        welcome += "/Tank_Percentage : Gets The Percentage of Water Filled in Tank\n";
        bot.sendMessage(chat_id, welcome, "");
      }
    }
  }
 
    
    
   
  
  
  
  
    
