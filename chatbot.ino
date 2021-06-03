#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>  
#include <ArduinoJson.h>

// DEFINE HERE THE KNOWN NETWORKS
const char* KNOWN_SSID[] = {"Xperia XZ1", "digitak_studio", "digitak"};
const char* KNOWN_PASSWORD[] = {"mamatnaga", "P4stiB1sa", "lampukantor"};

const int   KNOWN_SSID_COUNT = sizeof(KNOWN_SSID) / sizeof(KNOWN_SSID[0]); 

// Telegram BOT
#define BOTtoken "1132610268:AAF1DnP_7xjLKTRdu1E5ZtiUTROgzrCJ8xc"  
#define CHAT_ID "700829490"
#define SENSOR_PIN 36
#define POLLED
bool state;
bool lastState;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);


int botRequestDelay = 700;
unsigned long lastTimeBotRan;
const int ledPin = 23;
const int ledPin1 = 2;
bool ledState = HIGH;
//bool ledState1 = HIGH;

// Handle 
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
       String chat_id = String(bot.messages[i].chat_id);
//    if (chat_id != CHAT_ID){
//      bot.sendMessage(chat_id, "Unauthorized user", "");
//      continue;
 //   }
    
    // message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/on to turn GPIO ON \n";
      welcome += "/off to turn GPIO OFF \n";
      welcome += "/state to request current GPIO state \n";

      String keyboard = "[[{ \"text\": \"/on\", \"callback_data\": \"1\" }],\
                         [{ \"text\": \"/off\", \"callback_data\": \"2\" }],\
                         [{ \"text\": \"/state\", \"callback_data\": \"3\" }]]";
      bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboard,true);
    }

    if (text == "/on") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = LOW;
      //ledState1 = LOW;
      digitalWrite(ledPin, ledState);
      //digitalWrite(ledPin1, ledState1);
    }
    
    if (text == "/off") {
      bot.sendMessage(chat_id, "LED state set to OFF", "");
      ledState = HIGH;
      //ledState1 = HIGH;
      digitalWrite(ledPin, ledState);
      //digitalWrite(ledPin1, ledState1);
    }
    
    if (text == "/state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is ON", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  digitalWrite(ledPin, ledState);
//  digitalWrite(ledPin1, ledState1);
  pinMode(SENSOR_PIN, INPUT);
  //attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  state = false;
  lastState = false;
  boolean wifiFound = false;
  int i, n;

  Serial.begin(115200);

  // ----------------------------------------------------------------
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  // ----------------------------------------------------------------
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");

  // ----------------------------------------------------------------
  // WiFi.scanNetworks will return the number of networks found
  // ----------------------------------------------------------------
  Serial.println(F("scan start"));
  int nbVisibleNetworks = WiFi.scanNetworks();
  Serial.println(F("scan done"));
  if (nbVisibleNetworks == 0) {
    Serial.println(F("no networks found. Reset to try again"));
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // ----------------------------------------------------------------
  // if you arrive here at least some networks are visible
  // ----------------------------------------------------------------
  Serial.print(nbVisibleNetworks);
  Serial.println(" network(s) found");

  // ----------------------------------------------------------------
  // check if we recognize one by comparing the visible networks
  // one by one with our list of known networks
  // ----------------------------------------------------------------
  for (i = 0; i < nbVisibleNetworks; ++i) {
    Serial.println(WiFi.SSID(i)); // Print current SSID
    for (n = 0; n < KNOWN_SSID_COUNT; n++) { // walk through the list of known SSID and check for a match
      if (strcmp(KNOWN_SSID[n], WiFi.SSID(i).c_str())) {
        Serial.print(F("\tNot matching "));
        Serial.println(KNOWN_SSID[n]);
      } else { // we got a match
        wifiFound = true;
        break; // n is the network index we found
      }
    } // end for each known wifi SSID
    if (wifiFound) break; // break from the "for each visible network" loop
  } // end for each visible network

  if (!wifiFound) {
    Serial.println(F("no Known network identified. Reset to try again"));
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // ----------------------------------------------------------------
  // if you arrive here you found 1 known SSID
  // ----------------------------------------------------------------
  Serial.print(F("\nConnecting to "));
  Serial.println(KNOWN_SSID[n]);

  // ----------------------------------------------------------------
  // We try to connect to the WiFi network we found
  // ----------------------------------------------------------------
  WiFi.begin(KNOWN_SSID[n], KNOWN_PASSWORD[n]);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  // ----------------------------------------------------------------
  // SUCCESS, you are connected to the known WiFi network
  // ----------------------------------------------------------------
  Serial.println(F("WiFi connected, your IP address is "));
  Serial.println(WiFi.localIP());
}
  
//  // Connect to Wi-Fi
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(1000);
//    Serial.println("Connecting to WiFi..");
//  }
//  // Print ESP32 Local IP Address
//  Serial.println(WiFi.localIP());
//}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
 
  {
    // read the state of the sensor and see if it 
    // matches the last known state
    if((state = digitalRead(SENSOR_PIN)) != lastState)
    {
        // no match, change the state of the LED
        bot.sendMessage(CHAT_ID, "Motion detected!!", "");
        digitalWrite(ledPin1,state);
        Serial.println("Motion Detected");
        // remember this state as the last
        lastState = state;
        // sensor indicates active when it pulls
        // the input to 0.
        //Serial.println("polled - " + String(state ? "IDLE" : "ACTIVE"));
    }
    yield();
    delay(200);
}
}
