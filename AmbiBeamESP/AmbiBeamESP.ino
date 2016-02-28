#include <FastLED.h>

// NeoPixelBus


// Wifi
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

// Wifi Manager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// How many leds in your strip?
#define NUM_LEDS 440
// Where is it connected ?
#define DATA_PIN 5

// Define the array of leds
CRGB leds[NUM_LEDS];

#define localPort 2222
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define ACK 0x06

WiFiUDP Udp;
unsigned char state = 0;
uint16_t len = 0;
unsigned char brightness = 254;
uint16_t chunkPos = 0;
unsigned long  timeout;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println(F("Entered config mode"));
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

char packetBuffer[1330];

void setup() {
  Serial.begin(115200);
  
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect("ESP", "test");

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  
  Udp.begin(localPort);
}

void loop() {
int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
    
    // read the packet into packetBufffer
    uint16_t packetLen = Udp.read(packetBuffer, 1330);
    if (packetLen > 0) {
      packetBuffer[packetLen] = 0;
  
      // Serial.println("Contents:");
      // Serial.println(packetBuffer);
      char* p = &packetBuffer[0];
      switch(state){
          case 0: {
            timeout = millis();
            if(*(p++) == SOH)
            {
              len = (uint16_t) *(p++) << 8 | *(p++);
              Serial.print(F("len: "));
              Serial.println(len, DEC);
              
              brightness = *(p++);
              Serial.print(F("brightness: "));
              Serial.println(brightness, DEC);
              if(*(p++) == STX){
                Serial.println(F("got stx"));
                FastLED.setBrightness(brightness);
                chunkPos = 0;
                state++;
              } else { break; }
            } else { break; } 
          }
        case 1: {
          timeout = millis();
          int bytesToCopy = len * 3;
          Serial.print("bytesToCopy");
          Serial.println(bytesToCopy, DEC);
          char* pleds = (char*)leds;
                    
          memcpy ( pleds, p, bytesToCopy);

          if (chunkPos + bytesToCopy >= (len * 3))
          {
            Serial.println("Copy Done");
            Serial.println("show");
            FastLED.show();      
            state = 0;
          } else {
            chunkPos += bytesToCopy;
          }
          break;
        }
      }
    }
  }
  if(millis() - timeout > 250)
    {
      //Serial.println(F("Timeout"));
      state = 0;
    }
}
