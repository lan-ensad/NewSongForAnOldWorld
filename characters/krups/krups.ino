/*
SQUIRT    20
CAST      21
SDA       8
SCL       9
LED       2

! servo 0 PCA9685
*/

#include <Adafruit_NeoPixel.h>
//---------------------------
//        PCA9685
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

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
#define SQUIRTRELAY 20
#define CASTRELAY 21
#define SERVOPIN 7

#define NUMPIXELS 29
Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
// Servo t(SERVOPIN), 500, 2500, 10, 5000);

int brightness = 50;
int redVal, greenVal, blueVal = 125;
int squirtSpeed = 20;
int amplitude = 400;
int flashDel = 20;
int snakeDel = 20;
int pix = 0;
unsigned long prevMillisFlash, prevMillisSnake = 0;
bool flash, snake, ledOn, rdmColor, cast, squirt, flashIsUp, speakState;

//-------------------------------------
//              SPEAKING
void Speak(int servonum, int amp, bool t) {
  // if (t) {
  //   pwm.setPWM(servonum, 0, amp);
  // } else if (!t) {
  //   pwm.setPWM(servonum, 0, 500);
  // }
  pwm.setPWM(servonum, 0, random(amp));
}

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
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("Krups")) {
      Serial.println("connected");
      client.subscribe("krups_brightness");
      client.subscribe("krups_flash");
      client.subscribe("krups_flashDel");
      client.subscribe("krups_snake");
      client.subscribe("krups_snakeDel");
      client.subscribe("krups_rdmColor");
      client.subscribe("krups_ledOn");
      client.subscribe("krups_RedGreenBlue");
      client.subscribe("krups_squirt");
      client.subscribe("krups_cast");
      client.subscribe("krups_squirt-speed");
      client.subscribe("krups_speak");
      client.subscribe("krups_speak-amplitude");
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
  if (String(topic) == "krups_brightness") {
    brightness = msg.toInt();
  } else if (String(topic) == "krups_speak") {
    if (String(msg) == "true") {
      if (speakState) {
        speakState = false;
        Speak(0, amplitude, speakState);
      } else if (!speakState){
        speakState = true;
        Speak(0, amplitude, speakState);
      }
    }
  } else if (String(topic) == "krups_speak-amplitude") {
    amplitude = map(msg.toInt(), 0, 100, 350, 800);
  } else if (String(topic) == "krups_cast") {
    if (String(msg) == "true") {
      cast = true;
    } else if (String(msg) == "false") {
      cast = false;
    }
  } else if (String(topic) == "krups_squirt") {
    if (String(msg) == "true") {
      Squirt();
    }
  } else if (String(topic) == "krups_squirt-speed") {
    squirtSpeed = msg.toInt();
  } else if (String(topic) == "krups_flash") {
    snake = false;
    flash = true;
  } else if (String(topic) == "krups_flashDel") {
    flashDel = map(msg.toInt(), 0, 100, 1, 200);
  } else if (String(topic) == "krups_snake") {
    snake = true;
    flash = false;
  } else if (String(topic) == "krups_snakeDel") {
    snakeDel = map(msg.toInt(), 0, 100, 1, 200);
  } else if (String(topic) == "krups_rdmColor") {
    if (String(msg) == "true") {
      rdmColor = true;
    } else if (String(msg) == "false") {
      rdmColor = false;
    }
  } else if (String(topic) == "krups_ledOn") {
    if (String(msg) == "true") {
      ledOn = true;
      flash = false;
      snake = false;
    } else if (String(msg) == "false") {
      ledOn = false;
    }
  } else if (String(topic) == "krups_RedGreenBlue") {
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

//================================================
//                    LOOP
//================================================

void setup() {
  Serial.begin(115200);

  pinMode(SQUIRTRELAY, OUTPUT);
  pinMode(CASTRELAY, OUTPUT);

  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);

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

  Cry();

  if (ledOn) {
    if (flash) {
      Flash(flashDel, redVal, greenVal, blueVal, rdmColor);
      delay(100);
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