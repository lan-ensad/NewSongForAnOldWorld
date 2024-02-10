// https://www.pololu.com/product/2997
/*
o → VM
o → GND
o → VCC
o → OCC
o → EN
o → ENB
o → PWM2
o → PWM1
o → OCM
o → DIAG
*/
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

#define ENB 0  //HIGH to ON
#define PWM2 6
#define PWM1 5

int mSpeed = 125;
bool turn, cw = false;

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
      client.subscribe("clerge_speed");
      client.subscribe("clerge_turn");
      client.subscribe("clerge_reverse");
      client.subscribe("clerge_brake");
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
  if (String(topic) == "clerge_turn") {
    if (String(msg) == "true") {
      turn = true;
    } else if (String(msg) == "false") {
      turn = false;
    }
  } else if (String(topic) == "clerge_speed") {
    mSpeed = msg.toInt();
  } else if (String(topic) == "clerge_reverse") {
    cw = !cw;
  } else if (String(topic) == "clerge_brake") {
    if (String(msg) == "false") {
      Brake();
    }
  }
}


void setup() {
  Serial.begin(115200);

  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(ENB, OUTPUT);

  digitalWrite(ENB, LOW);  //turn off driver

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void Brake() {
  analogWrite(PWM1, 0);
  analogWrite(PWM2, 0);
}
void Stop() {
  digitalWrite(ENB, LOW);
}
void Forward(int sd) {
  digitalWrite(ENB, HIGH);
  analogWrite(PWM2, 0);
  analogWrite(PWM1, sd);
}
void Backward(int sd) {
  digitalWrite(ENB, HIGH);
  analogWrite(PWM1, 0);
  analogWrite(PWM2, sd);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (turn) {
    if (cw) {
      Forward(mSpeed);
    } else {
      Backward(mSpeed);
    }
  } else if (!turn) {
    Stop();
  }
}