#include <WiFi.h>
#include <PubSubClient.h>
#define LED_GREEN 14
#define LED_YELLOW 12
#define LED_RED 13

const char* WIFI_SSID = "EGN9265";
const char* WIFI_PASS = "5518UA19GR73ALD";
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

const char* CLIENT_ID = "test"; // unique client id
const char* IN_TOPIC = "ucb/practica/mqtt/juan";   // subscribe
const char* OUT_TOPIC = "ucb/practica/mqtt"; // publish

bool redLED = true;
bool yellowLED = true;
bool greenLED = true;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

int cm = 0;

long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  return pulseIn(echoPin, HIGH);
}

constexpr unsigned int str2int(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

void turnOnLed(String led){
  if(led == "1"){
    if(redLED)
      digitalWrite(LED_RED, HIGH);
    else
      digitalWrite(LED_RED, LOW);
    redLED = !redLED;
  }
  if(led == "2"){
    if(yellowLED)
      digitalWrite(LED_YELLOW, HIGH);
    else
      digitalWrite(LED_YELLOW, LOW);
    yellowLED = !yellowLED;
  }
  if(led == "3"){
    if(greenLED)
      digitalWrite(LED_GREEN, HIGH);
    else
      digitalWrite(LED_GREEN, LOW);
    greenLED = !greenLED;
  }
}
// PubSubClient callback function
void callback(const char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += String((char) payload[i]);
  }
  if (String(topic) == IN_TOPIC) {
    Serial.println("Message from topic " + String(IN_TOPIC) + ":" + message);
    turnOnLed(message);
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Couldn't connect to WiFi.");
    while(1) delay(100);
  }
  Serial.println("Connected to " + String(WIFI_SSID));

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
}

boolean mqttClientConnect() {
  Serial.println("Connecting to MQTT broker...");
  if (mqttClient.connect(CLIENT_ID)) {
    Serial.println("Connected to " + String(MQTT_BROKER));

    mqttClient.subscribe(IN_TOPIC);

    Serial.println("Subscribed to " + String(IN_TOPIC));
  } else {
    Serial.println("Couldn't connect to MQTT broker.");
  }
  return mqttClient.connected();
}

unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;

unsigned char counter = 0;

void loop() {
  cm = 0.01723 * readUltrasonicDistance(27, 26);
  unsigned long now = millis();
  if (!mqttClient.connected()) {
    // Connect to MQTT broker
    if (now - previousConnectMillis >= 5000) {
      previousConnectMillis = now;
      if (mqttClientConnect()) {
        previousConnectMillis = 0;
      } else delay(1000);
    }
  } else {
    // This should be called regularly to allow the client to process incoming messages and maintain its connection to the server
    mqttClient.loop();
    delay(100);

    if (now - previousPublishMillis >= 10000) {
      previousPublishMillis = now;

      // Publish message
      String strCounter = String(counter++);
      String message = String(cm);
      mqttClient.publish(OUT_TOPIC, message.c_str());
    }
  }
}
