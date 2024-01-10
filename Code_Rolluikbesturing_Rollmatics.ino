#include <WiFi.h>
#include <MQTTPubSubClient.h>
#include <ArxContainer.h>
#include <EEPROM.h>
#include <EEPROMRollingCodeStorage.h>
#include <SomfyRemote.h>

#define EMITTER_GPIO 2
#define EEPROM_ADDRESS 0

#define REMOTE 0x511234
#define REMOTE1 0x5184c8
#define REMOTE2 0x518abc
#define REMOTE3 0xc6c456
#define REMOTE4 0x59345

// Replace the next variables with your SSID/Password combination
const char* ssid = "ERBJHN";
const char* password = "EC4F2C5DA5C";

const char* mqtt_server = "192.168.2.3";

const char* address = "homeassistant/status"; // Replace with the MQTT topic you want to subscribe to
const int ledPin1 = 1;                        // Led1 voor programmeren rolluik 1
const int ledPin2 = 2;                        // led2 voor programmeren rolluik 2
const int digitalPin1 = 23;                   // PROG knop op IO 23

int digitalValue1 = digitalRead(digitalPin1); // Uitlezen van de programmeerknop

EEPROMRollingCodeStorage rollingCodeStorage(EEPROM_ADDRESS);
SomfyRemote somfyRemote(EMITTER_GPIO, REMOTE, &rollingCodeStorage);

// EEPROMRollingCodeStorage rollingCodeStorage1(0);
// EEPROMRollingCodeStorage rollingCodeStorage2(2);
// EEPROMRollingCodeStorage rollingCodeStorage3(4);
// EEPROMRollingCodeStorage rollingCodeStorage4(6);
// SomfyRemote somfyRemote1(EMITTER_GPIO, REMOTE1, &rollingCodeStorage1);
// SomfyRemote somfyRemote2(EMITTER_GPIO, REMOTE2, &rollingCodeStorage2);
// SomfyRemote somfyRemote3(EMITTER_GPIO, REMOTE3, &rollingCodeStorage3);
// SomfyRemote somfyRemote4(EMITTER_GPIO, REMOTE4, &rollingCodeStorage4);

WiFiClient client;
MQTTPubSubClient mqtt;

void setup() 
{
  Serial.begin(115200);

  somfyRemote.setup();

  // somfyRemote1.setup();
  // somfyRemote2.setup();
  // somfyRemote3.setup();
  // somfyRemote4.setup();
  
  digitalWrite(EMITTER_GPIO, LOW);
  
  #if defined(ESP32)
	if (!EEPROM.begin(4)) 
  {
		Serial.println("failed to initialise EEPROM");
		delay(1000);
	}
  #endif

  pinMode(digitalPin1, INPUT);          // programmeerknop instellen als input
  digitalWrite(digitalPin1, HIGH);      // programmeerknop hoog maken (pull-down)

  //Wi-Fi connection
  WiFi.begin(ssid, password);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  Serial.print("Connecting to WiFi...");
  delay(2000);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to WiFi!");

  Serial.print("Connecting to host...");
  delay(2000);
  while (!client.connect("192.168.2.3", 1883)) //core-mosquitto 
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to host!");

  mqtt.begin(client);

  Serial.print("Connecting to MQTT broker...");
  delay(2000);
  while (!mqtt.connect("Rolluiken", "mqtt_user", "mqtt_user")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to MQTT broker!");

  mqtt.subscribe(address, [](const String& payload, const size_t size) 
  {
    Serial.print(address);
    Serial.print(": ");
    Serial.println(payload);

digitalWrite(ledPin1, LOW);
digitalWrite(ledPin2, LOW);

    if (payload == "on") 
    {
      Serial.println("Turning the LED ON.");
      digitalWrite(ledPin1, HIGH);
    } 
    else if (payload == "off") 
    {
      Serial.println("Turning the LED OFF.");
      digitalWrite(ledPin1, LOW);
    } 
    else if (payload == "prog" || digitalValue1 == LOW) 
    {
      const Command command = getSomfyCommand("prog");
      somfyRemote.sendCommand(command);
      digitalWrite(ledPin1, HIGH);
    } 
    else if (payload == "prog2") 
    {
      const Command command = getSomfyCommand("prog");
      somfyRemote.sendCommand(command);
      digitalWrite(ledPin2, HIGH);
    } 
    
    else if (payload == "up") 
    {
      const Command command = getSomfyCommand("up");
      somfyRemote.sendCommand(command);
    }
    else if (payload == "down") 
    {
      const Command command = getSomfyCommand("down");
      somfyRemote.sendCommand(command);
    }
});
}

void loop() {
  mqtt.update(); // Update MQTT connection
  delay(10);
}