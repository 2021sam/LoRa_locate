// Repo:
// https://github.com/2021sam/LoRa_locate

// Setup:
//  Board:
//   Tools -> Boards -> Boards Manager -> esp32 -> install esp32 by Espressif Systems
//   Tools -> Board -> TTGO LoRa32-OLED
//   https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series

//  Libraries:
      // https://github.com/sandeepmistry/arduino-LoRa
      // https://github.com/olikraus/u8g2


#include <LoRa.h>
#include "boards.h"
#include "BluetoothSerial.h"

String recv1 = "";
String recv2 = "";
String recv3 = "";
#define ARRAYSIZE 1000
String gps_path[ARRAYSIZE] = {"uno", "duo", "tri"};

//#define USE_PIN // Uncomment this to use PIN during pairing. The pin is specified on the line below
const char *pin = "1234"; // Change this to more secure PIN.

String device_name = "ESP32-BT-GPS";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
int count_received = 0;


void bluetooth_setup() {
  Serial.begin(115200);
  SerialBT.begin(device_name); //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  //Serial.printf("The device with name \"%s\" and MAC address %s is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str(), SerialBT.getMacString()); // Use this after the MAC method is implemented
  #ifdef USE_PIN
    SerialBT.setPin(pin);
    Serial.println("Using PIN");
  #endif
}

void setup()
{
    initBoard();
    // When the power is turned on, a delay is required.
    delay(1500);
    Serial.println("LoRa Receiver");
    Serial.println(LoRa_frequency);
    Serial.println(RADIO_CS_PIN);
    Serial.println(RADIO_RST_PIN);
    Serial.println(RADIO_DIO0_PIN);
    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
    if (!LoRa.begin(LoRa_frequency)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    bluetooth_setup();
}

void transmit_Bluetooth(String recv)
{
  SerialBT.println("https://www.google.com/maps/dir//" + recv);
  delay(20);
}


void bluetooth_loop()
{
        String recv = "";
        char incoming_char = '\0';
        int c = 0;
        while (SerialBT.available()) {
          incoming_char = (char)SerialBT.read();
          if(incoming_char != '\n' && incoming_char != '\r')
          {
                      recv += incoming_char;
          // recv += (char)SerialBT.read();
          // Serial.println(recv);
          c++;

          }
        }
       
        if (recv.length())
        {
          Serial.print("c: ");
           Serial.println(c);
          //   Serial.print("Bluetooth: ");
          //   Serial.println(recv);

          if (recv.equals("clear"))
          {
            count_received = 0;
          }


          if (recv.equals("count"))
          {
              SerialBT.print("GPS pins: ");
              SerialBT.println(count_received);             
          }


            if (recv.equals("log"))
            // if (recv.substring(0, 3) == "log")
            {
              Serial.println("Satellites,HDOP,Time,Latitude,Longitude,Altitude_meters,speed_kmph");
              SerialBT.println("Satellites,HDOP,Time,Latitude,Longitude,Altitude_meters,speed_kmph");
              // SerialBT.println("Latitude,Longitude");
              for (int i = 0; i < count_received; i++){
                Serial.print(i);
                Serial.print(": ");
                Serial.println(gps_path[i]);
                SerialBT.println(gps_path[i]);
              }
            }
          
        }
}

void transmit_LoRa()
{
          Serial.println("transmit_LoRa");
          LoRa.beginPacket();
          LoRa.print("hello LoRa bany");
          // LoRa.print(count_received);
          LoRa.endPacket();
}


void loop()
{
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        String recv = "";
        while (LoRa.available()) {
            recv += (char)LoRa.read();
        }
        // Serial.println("Received packet:");
        if (recv.length() > 0)
        {
          // Serial.print("count_received:");
          gps_path[count_received] = recv;
          Serial.print(++count_received);
          Serial.print(": ");
          Serial.println(recv);

          delay(40);     //  May need a delay on this side to reset for a send or other side to reset to receive.  30 may work
          LoRa.beginPacket();
          LoRa.print("Confirm: ");
          LoRa.print(count_received);
          LoRa.endPacket();
          transmit_Bluetooth(recv);
        }

        
        recv3 = recv2;
        recv2 = recv1;
        recv1 = recv;

        // print RSSI of packet
        // Serial.print("' with RSSI ");
        // Serial.println(LoRa.packetRssi());
// #ifdef HAS_DISPLAY
        if (u8g2) {
            u8g2->clearBuffer();
            char buf[256];
            // u8g2->drawStr(0, 12, "Received OK!");
            u8g2->drawStr(0, 8, recv1.c_str());
            u8g2->drawStr(0, 26, recv2.c_str());
            u8g2->drawStr(0, 38, recv3.c_str());
            snprintf(buf, sizeof(buf), "RSSI:%i", LoRa.packetRssi());
            u8g2->drawStr(0, 64, buf);

            snprintf(buf, sizeof(buf), "SNR:%.1f", LoRa.packetSnr());
            u8g2->drawStr(70, 64, buf);
            u8g2->sendBuffer();
        }
// #endif
    // bluetooth_loop(recv);
    // transmit_Bluetooth(recv);
    }
    // transmit_LoRa();
    bluetooth_loop();
}
