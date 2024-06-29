

#include <Arduino.h>
#include<Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

WiFiMulti WiFiMulti;
SocketIOclient socketIO;
JsonDocument doc;
#define DHT11PIN 5     // what digital pin the DHT11 is conected to
#define DHTTYPE DHT22   // setting the type as DHT11
DHT dht(DHT11PIN, DHTTYPE);  //Enter your WIFI password
char jsonOutput[128];
#define USE_SERIAL Serial
// LiquidCrystal_I2C lcd(0x3F,16,2);
LiquidCrystal_I2C lcd(0x27,16,2);

float h =0.0;
float t = 0.0;

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            USE_SERIAL.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);

            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
        {
            char * sptr = NULL;
            int id = strtol((char *)payload, &sptr, 10);
            USE_SERIAL.printf("[IOc] get event: %s id: %d\n", payload, id);
            if(id) {
                payload = (uint8_t *)sptr;
               

            }
            
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload, length);
            if(error) {
                USE_SERIAL.print(F("deserializeJson() failed: "));
                USE_SERIAL.println(error.c_str());
                return;
            }

            String eventName = doc[0];
            USE_SERIAL.printf("[IOc] event name: %s\n", eventName.c_str());

            // Message Includes a ID for a ACK (callback)
            if(id) {
                // creat JSON message for Socket.IO (ack)
                DynamicJsonDocument docOut(1024);
                JsonArray array = docOut.to<JsonArray>();

                // add payload (parameters) for the ack (callback function)
                JsonObject param1 = array.createNestedObject();
                param1["now"] = millis();

                // JSON to String (serializion)
                String output;
                output += id;
                serializeJson(docOut, output);

                // Send event
                socketIO.send(sIOtype_ACK, output);
            }
        }
            break;
        case sIOtype_ACK:
            USE_SERIAL.printf("[IOc] get ack: %u\n", length);
            break;
        case sIOtype_ERROR:
            USE_SERIAL.printf("[IOc] get error: %u\n", length);
            break;
        case sIOtype_BINARY_EVENT:
            USE_SERIAL.printf("[IOc] get binary: %u\n", length);
            break;
        case sIOtype_BINARY_ACK:
            USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
            break;
    }
}

void setup() {
    //USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);
  //                  // initialize the lcd 
  // lcd.init();
  // Print a message to the LCD.
  Wire.begin();
  lcd.init();
  lcd.begin(16,2);
  lcd.backlight();
 
  lcd.setCursor(0,1);
  lcd.print("Welcome");
  

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }

    WiFiMulti.addAP("Takyon_WiFi", "t@ky0n@$");
    // WiFiMulti.addAP("Binod-4G", "b1b2bfree");

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    String ip = WiFi.localIP().toString();
    USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ip.c_str());

    // server address, port and URL
    socketIO.begin("192.168.0.137", 3000, "/socket.io/?EIO=4");
    // socketIO.begin("192.168.1.238", 3000, "/socket.io/?EIO=4");

    // event handler

    // event handler
    socketIO.onEvent(socketIOEvent);
    dht.begin();

}

unsigned long messageTimestamp = 0;
void loop() {
 
  dht.begin();
   h = dht.readHumidity();
  
   t = dht.readTemperature(false);  
  // delay(2000);
  if(!isnan(h) && !isnan(t)){
     Serial.print("Humidity = ");
    Serial.print(h);
    Serial.print("%  ");
    Serial.print("Temperature = ");
    Serial.print(t); 
    Serial.println("C  ");
    lcd.clear();
    lcd.print("Temperature = "+String(t));
    lcd.setCursor(0,1);
      lcd.print("Humidity = "+String(h));
       uint64_t now = millis();

    // if(now - messageTimestamp > 2000) {
    //     messageTimestamp = now;

        // creat JSON message for Socket.IO (event)
        DynamicJsonDocument doc(1024);
        JsonArray array = doc.to<JsonArray>();

        // add evnet name
        // Hint: socket.on('event_name', ....
        array.add("message");

        // add payload (parameters) for the event
        JsonObject payload = array.createNestedObject();
         payload["text"] = "Connected";
        payload["temperature"] = t;
payload["humidity"] = h;
payload["timestamp"] = now;
        //  USE_SERIAL.println(param1);

        // JSON to String (serializion)
        String output;
        serializeJson(doc, output);

        // Send event
        // socketIO.sendEVENT(output);
        socketIO.send(sIOtype_EVENT, output.c_str(), output.length());


  }
  
    socketIO.loop();

          
      
        

        // Print JSON for debugging
        // USE_SERIAL.println(output);
     
    
}
// void senseTemperature(){
//  dht.begin();
//   float h = dht.readHumidity();
//   delay(10);
//   float t = dht.readTemperature(false);  
  


//   if (!isnan(h) && !isnan(t)) {
//     Serial.print("Humidity = ");
//     Serial.print(h);
//     Serial.print("%  ");
//     Serial.print("Temperature = ");
//     Serial.print(t); 
//     Serial.println("C  ");
//     http.begin(server_url);
//       http.addHeader("Content-Type", "application/json");

//       String postData = "{\"temperature\":" + String(t) + ",\"humidity\":" + String(h) + "}";
      

//       int httpResponseCode = http.POST(postData);
//       if (httpResponseCode > 0) {
//         String response = http.getString();
//         Serial.println("HTTP Response code: " + String(httpResponseCode));
//         Serial.println("Response: " + response);
//       } else {
//         Serial.println("Error on HTTP request");
//       }
//       http.end();
//           // Sending data over client


//         delay(1000);
 
//   } else {
//     Serial.println("Failed to read from DHT sensor!");
//      Serial.print(h);
//   }     
// }