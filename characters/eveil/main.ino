//https://github.com/lpaseen/ht16k33
#include "ht16k33.h"
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "SSID";
const char* password = "PASS";
const char* mqttServer = "192.168.0.196";

const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

#define PINLED 0
HT16K33 HT;  //esp32-C3 → SDA=8 ; SCL=9
Fader fader = Fader(PINLED, 0, 255, 1, 1, 20, 20, 500, 1500);

const uint8_t I = 10;
const uint8_t J = 7;
const uint8_t NBSEG = 4;
const int MAXSPD = 5;
const int MINSPD = 250;

bool speech, light, speechIsUp = false;
int speechSpeed = 50;
int speechSegs = 2;
int faderTime = 20;
int faderMinBrightness = 0;
int faderMaxBrightness = 250;

unsigned int prevMillisFader;

uint8_t numCorrespond[] = { 6, 2, 5, 4, 3, 5, 6, 3, 7, 6 };
uint8_t numTab[I][J] = {
  { 0, 1, 2, 3, 4, 5, 8 },  //0
  { 1, 2, 8, 8, 8, 8, 8 },  //1
  { 0, 1, 3, 4, 6, 8, 8 },  //2
  { 0, 1, 2, 3, 6, 8, 8 },  //3
  { 1, 2, 5, 6, 8, 8, 8 },  //4
  { 0, 2, 3, 5, 6, 8, 8 },  //5
  { 0, 2, 3, 4, 5, 6, 8 },  //6
  { 0, 1, 2, 8, 8, 8, 8 },  //7
  { 0, 1, 2, 3, 4, 5, 6 },  //8
  { 0, 1, 2, 3, 5, 6, 8 }   //9
};
uint8_t allSeg[] = {
  0, 1, 2, 3, 4, 5, 6,         //7,//dot
  16, 17, 18, 19, 20, 21, 22,  //23,//dot
  //33, //double dots
  48, 49, 50, 51, 52, 53, 54,  //55,//dot
  64, 65, 66, 67, 68, 69, 70,  //71,//dot
};
uint8_t segs[4][7] = {
  { 0, 1, 2, 3, 4, 5, 6 },
  { 16, 17, 18, 19, 20, 21, 22 },
  { 48, 49, 50, 51, 52, 53, 54 },
  { 64, 65, 66, 67, 68, 69, 70 }
};

//================================================
//                WIFI SETTINGS
//================================================

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Eveil")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("eveil_speak");
      client.subscribe("eveil_speak-speed");
      client.subscribe("eveil_speak-maxSegs");
      client.subscribe("eveil_light");
      client.subscribe("eveil_light-speed");
      client.subscribe("eveil_light-minBri");
      client.subscribe("eveil_light-maxBri");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String msg;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    msg += (char)message[i];
  }
  Serial.println();
  if (String(topic) == "eveil_speak") {
    if (msg == "true") {
      speech = true;
    } else if (msg == "false") {
      speech = false;
    }
  } else if (String(topic) == "eveil_light") {
    if (msg == "true") {
      light = true;
    } else if (msg == "false") {
      light = false;
    }
  } else if (String(topic) == "eveil_speak-speed") {
    speechSpeed = map(msg.toInt(), 0, 100, MINSPD, MAXSPD);
    Serial.println(speechSpeed);
  } else if (String(topic) == "eveil_speak-maxSegs") {
    speechSegs = msg.toInt();
    Serial.println(speechSegs);
  } else if (String(topic) == "eveil_light-speed") {
    faderTime = map(msg.toInt(), 0, 100, 125, 1);
  } else if (String(topic) == "eveil_light-minBri") {
    faderMinBrightness = msg.toInt();
  } else if (String(topic) == "eveil_light-maxBri") {
    faderMaxBrightness = msg.toInt();
  }
}

void RandomSpeech(int del, int max) {
  if (millis() - prevMillisFader >= del) {
    prevMillisFader = millis();
    if (speechIsUp) {
      for (int i = 0; i < max; i++) {
        HT.setLedNow(allSeg[int(random(sizeof(allSeg)))]);
      }
      speechIsUp = false;
    } else {
      HT.clearAll();
      speechIsUp = true;
    }
  }
}

//================================================
//                    LOOP
//================================================


void setup() {
  Serial.begin(115200);

  HT.begin(0x00);

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (speech) {
    RandomSpeech(speechSpeed, speechSegs);
  } else if (!speech) {
    HT.clearAll();
  }

  if (light) {
    fader.setBrightness(faderMinBrightness, faderMaxBrightness);
    fader.setTimeTo(faderTime, faderTime);
    fader.check();
  } else if (!light) {
    fader.stopLed();
  }
}