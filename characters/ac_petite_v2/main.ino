#include <Adafruit_NeoPixel.h>
//---------------------------
//        NETWORK
#include <WiFi.h>
#include <PubSubClient.h>
const char* ssid = "SSID";
const char* password = "PASS";
const char* mqttServer = "192.168.0.196";
const int mqttPort = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

//---------------------------
//      DEFINITIONS
#define LEDPIN 2
#define SERVOPIN1 5
#define SERVOPIN2 7

#define NUMPIXELS 45

Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
//Servo(PIN, min_cycle, max_cycle, min_speed, max_speed);
Servo t(SERVOPIN1, 500, 2500, 10, 5000);   //COUVERCLE
Servo t2(SERVOPIN2, 500, 2500, 10, 5000);  //VOLETS

int brightness = 50;
int redVal, greenVal, blueVal = 125;
int flashDel = 20;
int snakeDel = 20;
int pix = 0;
unsigned long prevMillisFlash, prevMillisSnake = 0;
bool flash, snake, ledOn, rdmColor, flashIsUp;


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
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ac_petite")) {
      Serial.println("connected");
      client.subscribe("acp_brightness");
      client.subscribe("acp_flash");
      client.subscribe("acp_flashDel");
      client.subscribe("acp_snake");
      client.subscribe("acp_snakeDel");
      client.subscribe("acp_rdmColor");
      client.subscribe("acp_ledOn");
      client.subscribe("acp_RedGreenBlue");
      client.subscribe("acp_turn");
      client.subscribe("acp_speed");
      client.subscribe("acp_reverse");
      client.subscribe("acp_storeOpen");
      client.subscribe("acp_storeClose");
      client.subscribe("acp_topOpen");
      client.subscribe("acp_topClose");
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
  if (String(topic) == "acp_storeOpen") {
    if (String(msg) == "true") {
      StoreOpen();
    }
  } else if (String(topic) == "acp_storeClose") {
    if (String(msg) == "true") {
      StoreClose();
    }
  } else if (String(topic) == "acp_topOpen") {
    if (String(msg) == "true") {
      TopClose();
    }
  } else if (String(topic) == "acp_topClose") {
    if (String(msg) == "true") {
      TopOpen();
    }
  } else if (String(topic) == "acp_brightness") {
    brightness = msg.toInt();
  } else if (String(topic) == "acp_flash") {
    snake = false;
    flash = true;
  } else if (String(topic) == "acp_flashDel") {
    flashDel = map(msg.toInt(), 0, 100, 1, 200);
  } else if (String(topic) == "acp_snake") {
    snake = true;
    flash = false;
  } else if (String(topic) == "acp_snakeDel") {
    snakeDel = map(msg.toInt(), 0, 100, 1, 200);
  } else if (String(topic) == "acp_rdmColor") {
    if (String(msg) == "true") {
      rdmColor = true;
    } else if (String(msg) == "false") {
      rdmColor = false;
    }
  } else if (String(topic) == "acp_ledOn") {
    if (String(msg) == "true") {
      ledOn = true;
      flash = false;
      snake = false;
    } else if (String(msg) == "false") {
      ledOn = false;
    }
  } else if (String(topic) == "acp_RedGreenBlue") {
    int rl, gl, bl = 0;
    bool r, g, b = false;
    String red, green, blue;

    Serial.println(msg);
    //----------------------------------
    //      -1- MSG INCOME
    for (int i = 0; i < length; i++) {
      if ((char)msg[i] == 'r') {
        r = true;
      } else if ((char)msg[i] == 'g') {
        r = false;
        g = true;
      } else if ((char)msg[i] == 'b') {
        g = false;
        b = true;
      } else if ((char)msg[i] == 'a') {
        b = false;
      }
      if (r) {
        red += (char)msg[i];
      }
      if (g) {
        green += (char)msg[i];
      }
      if (b) {
        blue += (char)msg[i];
      }
    }
    //----------------------------------
    //       -2- FORMAT INCOME
    red.remove(0, 3);
    green.remove(0, 3);
    blue.remove(0, 3);
    rl = red.length();
    gl = green.length();
    bl = blue.length();
    red.remove(rl - 2, 2);
    green.remove(gl - 2, 2);
    blue.remove(bl - 2, 2);
    redVal = red.toInt();
    greenVal = green.toInt();
    blueVal = blue.toInt();
  }
}

//-------------------------------------
//              SERVO
void Speack(uint8_t min, uint8_t max, uint8_t s, int del) {
  // min target, max targat, speed, delay
  uint8_t target = uint8_t(random(min, max));
  t.Move(target, s);
  delay(5);
  t.Move(0, s);
  delay(del);
}
void StoreOpen() {
  t2.Move(180, 1);
}
void StoreClose() {
  t2.Move(120, 1);
}
void TopOpen() {
  t.Move(0, 1);
}
void TopClose() {
  t.Move(90, 1);
}
//-------------------------------------
//              LEDS
void Snake(int up, int R, int G, int B, boolean Rdm) {
  pixels.clear();
  if (millis() - prevMillisSnake >= up) {
    prevMillisSnake = millis();
    if (!Rdm) {
      pixels.setPixelColor(pix, pixels.Color(R, G, B));
      pixels.show();
      (pix >= NUMPIXELS) ? pix = 0 : pix++;
    } else {
      pixels.setPixelColor(pix, pixels.Color(random(255), random(255), random(255)));
      pixels.show();
      (pix >= NUMPIXELS) ? pix = 0 : pix++;
    }
  } else {
    pixels.clear();
  }
}
void Flash(int up, int R, int G, int B, boolean Rdm) {
  pixels.clear();

  if (millis() - prevMillisFlash >= up) {
    prevMillisFlash = millis();
    if (flashIsUp) {
      pixels.clear();
      flashIsUp = false;
    } else {
      if (!Rdm) {
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(R, G, B));
        }
      } else {
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(random(255), random(255), random(255)));
        }
      }
      flashIsUp = true;
    }
    pixels.show();
  }
}
void setup() {
  Serial.begin(115200);

  pixels.begin();
  pixels.setBrightness(brightness);

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  pixels.setBrightness(brightness);

  if (ledOn) {
    if (flash) {
      Flash(flashDel, redVal, greenVal, blueVal, rdmColor);
    } else if (snake) {
      Snake(snakeDel, redVal, greenVal, blueVal, rdmColor);
    }
  } else if (!ledOn) {
    pixels.clear();
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
  }
}