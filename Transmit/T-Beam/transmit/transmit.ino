// Setup:
//   Tools -> Boards -> Boards Manager -> esp32 -> install esp32 by Espressif Systems
//   Tools -> Board -> esp32 --> T-Beam
//   https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series

#include <LoRa.h>
#include "boards.h"
#include <TinyGPS++.h>
#include <TaskScheduler.h>

TinyGPSPlus gps;

int count_sent = 0;
int count_received = 0;
float latitude = 0;
float longitude = 0;



#define PERIOD1 1000
#define DURATION 10000
void transmit_sos();
Task sos ( 10000, TASK_FOREVER, &transmit_sos );
Scheduler runner;


void transmit_sos(){
    if (gps.location.isValid())
   {
    latitude = gps.location.lat();
    longitude = gps.location.lng();

    LoRa.beginPacket();
    LoRa.print(latitude, 6);
    LoRa.print(",");
    LoRa.print(longitude, 6);
    LoRa.endPacket();
  
    count_sent++;
    Serial.print("Sent packet: ");
    Serial.print(count_sent);
    update_display();
  }
}

void setup()
{
    initBoard();
    // When the power is turned on, a delay is required.
    delay(1500);
    Serial.println("LoRa Sender");
    Serial.println(LoRa_frequency);
    Serial.println(RADIO_CS_PIN);
    Serial.println(RADIO_RST_PIN);
    Serial.println(RADIO_DIO0_PIN);
    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);

    if (!LoRa.begin(LoRa_frequency)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    //  Sams GPS
    delay(1500);
    Serial.print(F("Testing TinyGPS++ library v. "));
    Serial.println(TinyGPSPlus::libraryVersion());
    Serial.println();

    runner.init();
    runner.addTask(sos);
    sos.enable();
}

void update_display(){
    if (u8g2) {
        char buf[256];
        u8g2->clearBuffer();
        snprintf(buf, sizeof(buf), "Sending: %d", count_sent);
        u8g2->drawStr(0, 11, buf);
        snprintf(buf, sizeof(buf), "Lat: %f", latitude, 6 );
        u8g2->drawStr(0, 30, buf);
        snprintf(buf, sizeof(buf), "Lon:%f", longitude, 6 );
        u8g2->drawStr(0, 45, buf);
        snprintf(buf, sizeof(buf), "Received: %d", count_received);
        u8g2->drawStr(0, 63, buf);
        u8g2->sendBuffer();
    }
}

void receive_LoRa_loop()
{
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
      String recv = "";
      while (LoRa.available()) {
          recv += (char)LoRa.read();
      }
      Serial.print(" --> Received packet:");
      Serial.println(recv);
      if (recv.length() > 0)
      {
        count_received++;
        update_display();
      }
  }
}


void gps_loop()
{
    // This sketch displays information every time a new sentence is correctly encoded.
    while (Serial1.available() > 0)
        if (gps.encode(Serial1.read()))
            // displayInfo();      // This may be need to be removed ?
            //  This gets the latest geo location.  Then you use the gps variable to update your global lat & long.

    if (millis() > 5000 && gps.charsProcessed() < 10) {
        Serial.println(F("No GPS detected: check wiring."));
        while (true);
    }
}

void displayInfo()
{
    Serial.print(F("Location: "));
    if (gps.location.isValid()) {
        Serial.print(gps.location.lat(), 6);
        Serial.print(F(","));
        Serial.print(gps.location.lng(), 6);
    } else {
        Serial.print(F("INVALID"));
    }
    Serial.println();
}


void loop(){
    gps_loop();
    // Serial.print("gps.location.isValid: ");
    // Serial.println(gps.location.isValid());
    if (gps.location.isValid())
   {
    receive_LoRa_loop();
    runner.execute();
   }
}
