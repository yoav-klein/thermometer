#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "config.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include "Adafruit_SHT31.h"

BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);

WiFiClientSecure net;
PubSubClient client(net);

const char* ssid = "Rimon";
const char* password = "yoavklein3";

Adafruit_SHT31 sht31 = Adafruit_SHT31();

const int led = 13;

time_t now;
time_t nowish = 1793313196;

void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(3 * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}


void connectClient() {
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);

  client.setServer(endpoint, 8883);
  
  while (!client.connect(client_name))
  {
    Serial.println(client.state());
    delay(1000);
  }

  Serial.println("Connected!!!");

  Serial.println(client.subscribe(topic));
 
}

void setup(void) {
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  if (! sht31.begin(0x44)) {   
    Serial.println("Check circuit. SHT31 not found!");
    while (1) delay(1);
  }


  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  
  NTPConnect();
  connectClient();

  Serial.println("HTTP server started");
}

void loop(void) {
  
  client.loop();
  if(!client.connected()) {
    connectClient();
  } else {
    Serial.println("loop");
    float temp = sht31.readTemperature();
    float hum = sht31.readHumidity();

    if (! isnan(temp)) { 
      Serial.print("Temperature(°C): "); 
      Serial.print(temp); 
      Serial.print("\t\t");
    } else { 
      Serial.println("Failed to read temperature!");
    }
    
    if (! isnan(hum)) {
      Serial.print("Humidity(%): "); 
      Serial.println(hum);
    } else { 
      Serial.println("Failed to read humidity!");
    }

    JsonDocument doc;
    doc["temp"] = temp;
    doc["hum"] = hum;

    char buffer[50];
    serializeJson(doc, buffer);

    client.publish(topic, buffer);
  }

  // delay(1000);
  Serial.println("Going to sleep");
  ESP.deepSleep(600e6); 

}
