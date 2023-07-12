//library needed for the DHT11 sensor
#include <DFRobot_DHT11.h>
//library needed for Wi-Fi connection
#include <WiFi.h>
//libraries needed for the MQTT broker
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

//variables needed for establishing the Wi-Fi connection
const char *SSID = "ssid";
const char *PWD = "pass";

//variables needed for the LDR
#define LDRpin 19
int lightIntensity = 0;

//variables needed for the DHT11 sensor
DFRobot_DHT11 DHT;
#define DHT_PIN 13
int temperature = 0;
int humidity = 0;

//variables needed for the soil moisture sensor
#define SOIL_PIN 12
int soilMoisture = 0;

// MQTT client
const char* mqtt_server = "mqtt server"; // replace with your broker url
const char* mqtt_username = "usr"; // replace with your Credential
const char* mqtt_password = "pass";
const int mqtt_port = 8883;

WiFiClientSecure espClient;  
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

//deep sleep time (in minutes)
#define DEEP_SLEEP_TIME 1

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

String topic;
String messageTemp;
String message;
void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];
  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);
  messageTemp = topic;
  Serial.print(messageTemp);
  //==============MESSAGE RECEPTION HERE (Uncomment to edit)====================
  /* if (messageTemp == "MESSAGE INPUT"){
    YOUR VARIABLE = incommingMessage.to**WHAT YOU NEED**();
  }*/
}



//custom function for putting the esp to sleep
void goToSleep() {
  Serial.println("Honk mimimimimi");
  //set a timer for waking up the device after one minute
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME * 60 * 1000000);
  //start the deep sleep mode

  esp_deep_sleep_start();
}

//custom function for connecting to the Wi-Fi network
void connectToWiFi() {
  Serial.print("Connectig to ");
 
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected.");
}

//custom function for connecting to the MQTT broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client- ";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      //SUBSCRIBE TO TOPIC HERE

     // client.subscribe("SUBSCRIBE NAME");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LDRpin, INPUT);
  char* data = "";
  Serial.begin(9600);

  connectToWiFi();

  //TAKING MEASUREMENTS FROM THE SENSORS

  //reading the ldr values
  lightIntensity = analogRead(LDRpin);
  lightIntensity = map(lightIntensity, 0, 4095, 100, 0);
  Serial.print("Light Intensity = ");
  Serial.println(lightIntensity);

  //reading the dht11 values
  DHT.read(DHT_PIN);
  temperature = DHT.temperature;
  humidity = DHT.humidity;
  Serial.print("Temperature = ");
  Serial.println(temperature);
  Serial.print("Humidity = ");
  Serial.println(humidity);

  //reading the soil moisture sensor
  soilMoisture = analogRead(SOIL_PIN);
  Serial.print("Soil Moisture = ");
  Serial.println(soilMoisture);

  //SENDING DATA
  while (!Serial) delay(1);
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  if (!client.connected()) reconnect();

  publishMessage("/plant/temperature",String(temperature),true);
  publishMessage("/plant/humidity",String(humidity),true);
  publishMessage("/plant/light_intensity",String(lightIntensity),true);
  publishMessage("/plant/soil_moisture",String(soilMoisture),true);

  goToSleep();

}

void loop() {
  
}

void publishMessage(const char* topic, String payload , boolean retained){
  if (client.publish(topic, payload.c_str(), true))
      Serial.println("Message publised ["+String(topic)+"]: "+payload);
}
