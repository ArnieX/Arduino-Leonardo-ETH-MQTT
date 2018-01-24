#include <Arduino.h>
#include <Ethernet2.h>
#include <PubSubClient.h>

//#define DEBUG

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.println(x)
#else
 #define DEBUG_PRINT(x)
#endif

// Network defaults
char mqtt_server[40]    = "10.0.1.4";   // Public broker
char mqtt_port[6]       = "1883";
char mqtt_username[20]  = "";
char mqtt_password[40]  = "";
char home_name[40]      = "homeprague";
char room[40]           = "livingroom";
char device_name[40]    = "smartcrumbs-one";

// MQTT Constants
String mqtt_devicestatus_set_topic     = String(home_name)+"/"+String(room)+"/"+String(device_name)+"/devicestatus";
String mqtt_test_topic                 = String(home_name)+"/"+String(room)+"/"+String(device_name);
String mqtt_test_set_topic             = String(home_name)+"/"+String(room)+"/"+String(device_name)+"/status";
String mqtt_pingall_get_topic          = String(home_name)+"/pingall";
String mqtt_pingallresponse_set_topic  = String(home_name)+"/pingallresponse";
String mqtt_pingall_response_text      = "{\""+String(room)+"/"+String(device_name)+"\":\"connected\"}";

// Global
long lastReconnectAttempt = 0;
long lastPublish = 0;

// Enter a MAC address for your controller below.
// SmartCrumbs uses WIZnet W5500 chip so we recommend using their MAC pool 00:08:DC:??:??:??
byte mac[] = { 0x00, 0x08, 0xDC, 0x00, 0x00, 0x01 };

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(10, 0, 1, 199);

// Initialize the Ethernet client library
EthernetClient ethClient;
PubSubClient client(ethClient);

void blink() {

digitalWrite(13, HIGH);
delay(20);
digitalWrite(13, LOW);

}

void mqttConnected() {

  DEBUG_PRINT("MQTT Connected");

  // Once connected, publish an announcement...
  client.publish(mqtt_devicestatus_set_topic.c_str(), "connected");
  // ... and resubscribe
  client.subscribe(mqtt_test_topic.c_str());
  client.subscribe(mqtt_pingall_get_topic.c_str());

}

boolean reconnect() {

  // MQTT reconnection function

    // Create a random client ID
    String clientId = "SmartCrumbs-Client-";
    clientId += String(random(0xff), HEX)+":"+String(random(0xff), HEX)+":"+String(random(0xff), HEX)+":"+String(random(0xff), HEX)+":"+String(random(0xff), HEX)+":"+String(random(0xff), HEX);

    DEBUG_PRINT(clientId);

    // Attempt to connect
    if ( strcmp(mqtt_username,"") == 0 || strcmp(mqtt_password,"") == 0 ) {

      if ( client.connect(clientId.c_str(), mqtt_devicestatus_set_topic.c_str(), 0, false, "disconnected") ) {

        mqttConnected();

      }

    } else {

      if ( client.connect(clientId.c_str(), (char*)mqtt_username, (char*)mqtt_password, mqtt_devicestatus_set_topic.c_str(), 0, false, "disconnected") ) {

        mqttConnected();

      }

    }

    return client.connected();

}

void callback(char* topic, byte* payload, unsigned int length) {

    char c_payload[length];
    memcpy(c_payload, payload, length);
    c_payload[length] = '\0';

    String s_topic = String(topic);         // Topic
    String s_payload = String(c_payload);   // Message content

    DEBUG_PRINT("MQTT Received");
    blink();

  // Handling incoming MQTT messages

    if ( s_topic == mqtt_test_topic ) {

      client.publish(mqtt_test_set_topic.c_str(), "1"); //respond, just for test

    } else if ( s_topic == mqtt_pingall_get_topic.c_str() ) {

      client.publish(mqtt_pingallresponse_set_topic.c_str(), mqtt_pingall_response_text.c_str());
      client.publish(mqtt_devicestatus_set_topic.c_str(), "connected");

      DEBUG_PRINT("MQTT Ping Received and handled");

    }


}

void setup() {

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  /*while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }*/

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    DEBUG_PRINT("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  DEBUG_PRINT("connecting...");
  DEBUG_PRINT(Ethernet.localIP());

  client.setServer((char*)mqtt_server, atoi(&mqtt_port[0]));
  client.setCallback(callback);

}

void loop() {

    if (!client.connected()) {
      DEBUG_PRINT("Disconnected from MQTT");
      long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        // Attempt to reconnect
        if (reconnect()) {
          lastReconnectAttempt = 0;
        }
      }
    } else {
      // Client connected
      client.loop();
    }

}
