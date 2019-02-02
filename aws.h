#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "Secret.h"

const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_SSID_PASSWORD;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;
String uid;

String get_uid(){
    uint8_t mac[6];
    String uid;

    WiFi.macAddress(mac);

    for (int i=5; i>=0; i--){
        uid += String(mac[i], HEX);
        Serial.print(i);
        Serial.println(": " + String(mac[i], HEX));
    }

    return uid;
}

void connectWiFi() {
    Serial.print("Attempting to connect to SSID: ");
    Serial.print(ssid);
    Serial.print(" ");

    while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
        // failed, retry
        Serial.print(".");
        delay(5000);
    }
    Serial.println();

    Serial.println("You're connected to the network");
    Serial.println();
}

void connectMQTT() {
    Serial.print("Attempting to MQTT broker: ");
    Serial.print(broker);
    Serial.println(" ");

    int attempts = 0;
    while (!mqttClient.connect(broker, 8883)) {
        // failed, retry
        Serial.print(".");
        delay(attempts*20 + 5);
        attempts++;
        if (attempts > 250){
            attempts = 250;
        }
    }
    Serial.println();

    Serial.println("You're connected to the MQTT broker");
    Serial.println();

    // subscribe to a topic
    mqttClient.subscribe("arduino/incoming");
}

void awsHandle(){
    // Do everything to establish and maintain an aws connection
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }

    if (!mqttClient.connected()) {
        // MQTT client is disconnected, connect
        connectMQTT();
    }

    // poll for new MQTT messages and send keep alives
    mqttClient.poll();
}

unsigned long getTime() {
    // get the current time from the WiFi module  
    return WiFi.getTime();
}

void awsPublishMessage(JsonObject& root) {
    // root["deviceID"] = uid;

    Serial.println("Publishing message");

    // String msg;
    // root.printTo(msg);
    
    root.printTo(Serial);
    Serial.println("\n");

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage("arduino/outgoing");
    // mqttClient.print("hello ");
    // mqttClient.print(getTime());
    // mqttClient.print(millis());
    // mqttClient.print("{\"SerialNumber\":");
    // mqttClient.print("\"" + uid + "\"");
    // mqttClient.print(", \"time\":");
    // mqttClient.print(millis());
    // mqttClient.print("}");

    // Send json object to mqtt print
    root.printTo(mqttClient);

    mqttClient.endMessage();
    
}

void onMessageReceived(int messageSize) {
    // we received a message, print out the topic and contents
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");

    // use the Stream interface to print the contents
    while (mqttClient.available()) {
        Serial.print((char)mqttClient.read());
    }
    Serial.println();

    Serial.println();
}


void awsSetup(){
    while (!Serial);

    if (!ECCX08.begin()) {
        Serial.println("No ECCX08 present!");
        while (1);
    }

    // Set a callback to get the current time
    // used to validate the servers certificate
    ArduinoBearSSL.onGetTime(getTime);

    // Set the ECCX08 slot to use for the private key
    // and the accompanying public certificate for it
    sslClient.setEccSlot(0, certificate);

    // Optional, set the client id used for MQTT,
    // each device that is connected to the broker
    // must have a unique client id. The MQTTClient will generate
    // a client id for you based on the millis() value if not set
    //
    // mqttClient.setId("clientId");
    uid = get_uid();
    // mqttClient.setId(uid);

    // Set the message callback, this function is
    // called when the MQTTClient receives a message
    mqttClient.onMessage(onMessageReceived);
    
    Serial.println("Setup Complete");
    Serial.print("mac address: ");
    Serial.println(uid);
}
