#include <AccelStepper.h>
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
#define EN 0
#define stepPin 5
#define dirPin 7
#define minSpeed 100
#define maxSpeed 1000

#define NUMPIXELS 90

Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

int brightness = 50;
int redVal, greenVal, blueVal = 125;
int flashDel = 20;
int snakeDel = 20;
int pix = 0;
unsigned long prevMillisFlash, prevMillisSnake = 0;
bool flash, snake, ledOn, rdmColor, flashIsUp;

int mSpeed = 150;
bool turn, reverse = false;

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
    if (client.connect("ac_grande")) {
      Serial.println("connected");
      client.subscribe("ac_brightness");
      client.subscribe("ac_flash");
      client.subscribe("ac_flashDel");
      client.subscribe("ac_snake");
      client.subscribe("ac_snakeDel");
      client.subscribe("ac_rdmColor");
      client.subscribe("ac_ledOn");
      client.subscribe("ac_RedGreenBlue");
      client.subscribe("ac_turn");
      client.subscribe("ac_speed");
      client.subscribe("ac_reverse");
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
  if (String(topic) == "ac_brightness") {
    brightness = msg.toInt();
  } else if (String(topic) == "ac_turn") {
    if (String(msg) == "true") {
      turn = true;
    } else if (String(msg) == "false") {
      turn = false;
    }
  } else if (String(topic) == "ac_speed") {
    mSpeed = map(msg.toInt(), 0, 100, minSpeed, maxSpeed);
  } else if (String(topic) == "ac_reverse") {
    mSpeed *= -1;
  } else if (String(topic) == "ac_flash") {
    snake = false;
    flash = true;
  } else if (String(topic) == "ac_flashDel") {
    flashDel = map(msg.toInt(), 0, 100, 1, 200);
  } else if (String(topic) == "ac_snake") {
    snake = true;
    flash = false;
  } else if (String(topic) == "ac_snakeDel") {
    snakeDel = map(msg.toInt(), 0, 100, 1, 200);
  } else if (String(topic) == "ac_rdmColor") {
    if (String(msg) == "true") {
      rdmColor = true;
    } else if (String(msg) == "false") {
      rdmColor = false;
    }
  } else if (String(topic) == "ac_ledOn") {
    if (String(msg) == "true") {
      ledOn = true;
      flash = false;
      snake = false;
    } else if (String(msg) == "false") {
      ledOn = false;
    }
  } else if (String(topic) == "ac_RedGreenBlue") {
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

void setup() {
  Serial.begin(115200);

  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);

  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(200);
  stepper.setCurrentPosition(0);

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

  if (turn) {
    digitalWrite(EN, LOW);
    stepper.setSpeed(mSpeed);
    stepper.run();
  } else if (!turn) {
    stepper.stop();
    digitalWrite(EN, HIGH);
  }
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