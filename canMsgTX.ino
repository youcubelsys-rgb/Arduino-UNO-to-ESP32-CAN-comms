#include <DHT.h>
#include <mcp2515.h>
#include <SPI.h>

DHT dht(21, DHT11);

struct can_frame canMsg;
struct MCP2515 mcp2515(5); // Pin on ESP32 which CS is connected to

#define MAX_RETRIES 3
#define CAN_ACK_ID 0x037



void setup() {
  Serial.begin(115200);
  SPI.begin();
  // Defining CAN message parameters
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  dht.begin();
}

void loop() {
  delay(2000); // DHT11 needs ~2s between reads

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Celsius
  int tempInt = (int)(temperature * 100); // Convert temperature from floating point to integer
  int humidInt = (int)(humidity * 100); // Same for humidity

  canMsg.can_id = 0x036; // CAN Message ID
  canMsg.can_dlc = 4; // Data length code (number of bytes)

  //Split data into 4 bytes 
  canMsg.data[0] = (tempInt >> 8) & 0xFF; // MSB of temperature 
  canMsg.data[1] = tempInt & 0xFF; //LSB of temperature
  canMsg.data[2] = (humidInt >> 8) & 0xFF; //MSB of humidity
  canMsg.data[3] = humidInt & 0xFF; //LSB of humidity

  bool messageSent = false;
  int retries = 0;

  while (!messageSent && retries < MAX_RETRIES) {
    if (mcp2515.sendMessage(&canMsg) == MCP2515::ERROR_OK){
      Serial.print("Temperature sent: ");
      Serial.print(temperature);
      Serial.println("°C");

      Serial.print("Humidity sent: ");
      Serial.print(humidity);
      Serial.println("%");

      // Wait for acknowledgement
      unsigned long startTime = millis();
      bool ackReceived = false;

      while (millis() - startTime < 500) { // Wait up to 500ms for ACK
        if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK){
          if (canMsg.can_id == CAN_ACK_ID) {
            ackReceived = true;
            break;
          }
        }
      }

      if (ackReceived) {
        Serial.println("ACK received");
        messageSent = true;
      } else{
        Serial.println("ACK not received, retrying...");
        retries++;
      }

    }

  }

  if (!messageSent) {
      Serial.println("Failed to send message after retries");
    }
    delay(1000);

}









