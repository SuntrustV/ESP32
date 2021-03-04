// https://shiftr.io/try.
//

#include <WiFi.h>
#include <MQTT.h>

const char ssid[] = "TP-LINK_29DAFC";
const char pass[] = "44459055";

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {

  Serial.print("Connecting WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nWiFi - Ok!");

  Serial.print("\nConnecting MQTT broker ");
  while (!client.connect("izi", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nMQTT borker - Ok!");

  client.subscribe("/izipoint/T1");
  client.subscribe("/izipoint/T2");
  client.subscribe("/izipoint/H1");
  client.subscribe("/izipoint/H2");

}

void messageReceived(String &topic, String &payload) {
  Serial.println("Incoming message: " + topic + " = " + payload);
}

void setup() {

  Serial.begin(9200);
  WiFi.begin(ssid, pass);
  client.begin("broker.shiftr.io", net);
  client.onMessage(messageReceived);
  connect();
}

void loop() {
  client.loop();
  delay(10);  // Для стабильной работы WiFi задержка 10мс

  if (!client.connected()) {
    connect();
  }


  if (millis() - lastMillis > 1000) {

    lastMillis = millis();

    client.publish("/izipoint/T1", String(random(10, 24)));
    client.publish("/izipoint/T2", String(random(10, 24)));
    client.publish("/izipoint/H1", String(random(10, 80)));
    client.publish("/izipoint/H2", String(random(10, 80)));
  }

}
