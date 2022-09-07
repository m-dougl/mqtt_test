#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define led_pin 12
/* Temperature and humidity sensor configuration*/
#define dht_pin 13
#define dht_type DHT11
DHT dht(dht_pin, dht_type);

float read_temperature(void){
  float temperature = dht.readTemperature();
  float result;
  if (! (isnan(temperature)))
    result = temperature;
  else
    result = -99.99;
  return result;
}
float read_humidity(void){
  float humidity = dht.readHumidity();
  float result;
  if (! (isnan(humidity)))
    result = humidity;
  else
    result = -99.99;
  return result;
}

/* Message Queuing Telemetry Transport (MQTT) and WiFi configuration*/
const char* SSID        = "Douglas";
const char* PASSWORD    = "fpi1Kwss";
const char* BROKER_MQTT = "broker.hivemq.com";
int BROKER_PORT         = 1883;
#define ID_MQTT                     "douglas_maia_mqtt"
#define topic_subscribe_led         "topic_on_off_led"
#define topic_publisher_temperature "topic_temperature"
#define topic_publisher_humidity    "topic_humidity"

WiFiClient espClient;
PubSubClient MQTT(espClient);
/*WiFi*/
void init_wifi(void){
  delay(50);
  Serial.print("Connecting to");
  Serial.print(SSID);
  Serial.println("Please wait. . .");
  WiFi.begin(SSID, PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print('.');
  }
  Serial.println();
  Serial.print("Connected!");
  Serial.println("IP: ");
  Serial.print(WiFi.localIP());
}
void reconnect_wifi(void){
  if (WiFi.status() == WL_CONNECTED)
    return;
  init_wifi();
}

/*MQTT*/
void mqtt_callback(char* topic, byte* payload, unsigned int length){
  String msg;
  for(int i = 0; i < length; i++){
    char c = (char)payload[i];
    msg += c;
  }
  Serial.print("Message received: ");
  Serial.println(msg);

  if (msg.equals("1")){
    digitalWrite(led_pin, HIGH);
    Serial.print("LED ON!!");
  }

  if (msg.equals("0")){
    digitalWrite(led_pin, LOW);
    Serial.print("LED OFF!!");
  }
}
void init_mqtt(void){
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}
void reconnect_mqtt(void){
  while(!MQTT.connected()){
    Serial.print("Trying reconnect to MQTT Broker...");
    if (MQTT.connect(ID_MQTT)){
      Serial.println("Successfully connected");
      MQTT.subscribe(topic_subscribe_led);
    }
    else{
      Serial.println("Connection failed!");
      Serial.println("Please wait 2 seconds...");
      delay(2000);
    }
  }

}

void setup() {
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  
  dht.begin();
  init_wifi();
  init_mqtt();
}

void loop() {
  char temperature[10] = {0};
  char humidity[10]    = {0};
  reconnect_mqtt();
  reconnect_wifi();

  sprintf(temperature, ".2fC", read_temperature());
  sprintf(humidity, ".2f", read_humidity());

  MQTT.publish(topic_publisher_temperature, temperature);
  MQTT.publish(topic_publisher_humidity, humidity);

  MQTT.loop();
  delay(2000);
}

