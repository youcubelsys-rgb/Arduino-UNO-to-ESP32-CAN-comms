#include <mcp2515.h>
#include <SPI.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(3,4,5,6,7,8);

struct can_frame canMsg;
MCP2515 mcp2515(10); // CS pin connection

#define CAN_ACK_ID 0x037 // CAN ID for acknowledgment



void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
lcd.begin(16,2);
SPI.begin();

lcd.setCursor(0,0);
lcd.print("LCD OK");

mcp2515.reset();
mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
mcp2515.setNormalMode();
}

void loop() {
  // put your main code here, to run repeatedly:

  int result = mcp2515.readMessage(&canMsg);  

  if (result == MCP2515::ERROR_OK) {

    if (result == MCP2515::ERROR_OK) {
      Serial.print("Received CAN ID: 0x");
      Serial.println(canMsg.can_id, HEX);
    } else {
        Serial.print("CAN read error: ");
        Serial.println(result); // prints the error code
      }

    if (canMsg.can_id == 0x036){

      int tempInt = (canMsg.data[0] << 8) | canMsg.data[1]; // Combine MSB and LSB
      float temperature = tempInt / 100.0; // Convert back to float

      int humidInt = (canMsg.data[2] << 8) | canMsg.data[3];
      float humidity = humidInt / 100.0 ;

      Serial.print("Temperature received: ");
      Serial.print(temperature);
      Serial.println("°C");

      Serial.print("Humidity received: ");
      Serial.print(humidity);
      Serial.println("%");

      lcd.setCursor(0,0);
      lcd.print("Temp: ");
      lcd.setCursor(10,0);
      lcd.print(temperature);

      lcd.setCursor(0,1);
      lcd.print("Humidity: ");
      lcd.setCursor(10,1);
      lcd.print(humidity);

      // Send acknowledgment to ESP32
      canMsg.can_id = CAN_ACK_ID;
      canMsg.can_dlc = 0; // No data needed for acknowledgement
      mcp2515.sendMessage(&canMsg);
      Serial.println("ACK Message Sent");
      
    }   
  }
}
