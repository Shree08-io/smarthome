  // This include is for the AWS IOT library that we installed
#include <AWS_IOT.h>
// This include is for Wifi functionality
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries (use the correct version)
#include <StreamString.h>

#include <PubSubClient.h>  //for mqtt 
////////////////
WiFiMulti wifiMulti;
 WebSocketsClient webSocket;
// prototypes

const char* mqtt_server = "broker.mqtt-dashboard.com";  //for MQTT

//for MQTT
WiFiClient espClient;
PubSubClient client1(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
////////////

#define MyApiKey "  " // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard  

#define DEVICE1 " "  //TODO: Device ID of first device
 
const int relayPin1 = 2; // TODO: Change according to your board
bool isConnected = false;
boolean wifiConnected = false;
//////////////////////
// Declare an instance of the AWS IOT library
AWS_IOT hornbill;

// Wifi credentials
char WIFI_SSID[]= "SSID";
char WIFI_PASSWORD[]="PASSWORD";

// Thing details
char HOST_ADDRESS[]="";   //your AWS IoT  Host name
char CLIENT_ID[]= "";     //client ID Thing-name
char TOPIC_NAME[]= " ";   //topic from aws 

// Connection status
int status = WL_IDLE_STATUS;
// Payload array to store thing shadow JSON document
char payload[512];
// Counter for iteration
int gas_value = 890;
int sendMessageBit =1;

// Pin for the button
const int INTERRUPT_PIN = 15;

// State for the button press
volatile byte state = LOW;

///////////////////
void turnOn(String deviceId) {
  if (deviceId == DEVICE1)
  {  
    Serial.print("Turn on device id: ");
    Serial.println(deviceId);
    
     digitalWrite(relayPin1, HIGH); // turn on relay with voltage HIGH
  }
}


 void turnOff(String deviceId) {
   if (deviceId == DEVICE1)
   {  
     Serial.print("Turn off Device ID: ");
     Serial.println(deviceId);
     
     digitalWrite(relayPin1, LOW);  // turn off relay with voltage LOW
   }
 }


  void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // For Switch or Light device types
//         {"deviceId": DEVICE1, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html

        // For Light device type
        // Look at the light example in github
          
        DynamicJsonDocument doc(2048);
        DeserializationError err = deserializeJson(doc,(char*)payload);
        String deviceId = doc["deviceId"];   
        Serial.println(err.c_str());
        String action = doc["action"];
     if(action == "setPowerState") { // Switch or Light
            String value = doc["value"];
            if(value == "ON") {
                turnOn(deviceId);
                Serial.printf("test1");
            } else {
                turnOff(deviceId);
                Serial.printf("test2");
            }
        }
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}
//////////////////

//for MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(2, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
void reconnect() {
  // Loop until we're reconnected
  while (!client1.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client1.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client1.publish("outTopic", "hello world");
      // ... and resubscribe
      client1.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client1.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//////////


void setup()
{
  WiFi.disconnect(true);
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  // initialise AWS connection
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Wifi network: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(5000);
  }
  Serial.println("Connected to Wifi!");
  if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)== 0) {
    Serial.println("Connected to AWS, bru");
    delay(1000);
  }
  else {
    Serial.println("AWS connection failed, Check the HOST Address");
    while(1);
  }
  // Set the button bin to a specific mode
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  // Attach the method to call when the button is pressed
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), buttonPress, CHANGE);

  ////////////
  pinMode(relayPin1, OUTPUT); 

  // Waiting for Wifi connect
  
   Serial.println("Connecting Wifi...");
    
    

  webSocket.begin("iot.sinric.com", 80, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);
  //for MQTT
  client1.setServer(mqtt_server, 1883);
  client1.setCallback(callback);
  /////
}


void loop() {
    if (sendMessageBit == 1) {
      sendMessage();
      sendMessageBit = 1;
    }

    ///////
   webSocket.loop();
  
  if(isConnected) {
      uint64_t now = millis();  
  }


   //for MQTT
   if (!client1.connected()) {
    reconnect();
  }
  client1.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "gas Value #%ld", gas_value );
    Serial.print("Publish message: ");
    Serial.println(msg);
    client1.publish("/gas", msg );
  }
   ///////
}  


void buttonPress() {
  state = !state;
  if (state == HIGH) {
    Serial.println("Button pressed");
    sendMessageBit = 1;
  }
  if (state == LOW) {
    Serial.println("Button released");
  }
}

void sendMessage() {
  gas_value++;
  sprintf(payload,"{\"state\":{\"reported\":{\"counter\":\" %d \"}}}",gas_value);
  Serial.println(payload);
  if(hornbill.publish(TOPIC_NAME,payload) == 0) {
    Serial.println("Message was published successfully");
  }
  else {
    Serial.println("Message was not published");
  }
  delay(5000);
}
