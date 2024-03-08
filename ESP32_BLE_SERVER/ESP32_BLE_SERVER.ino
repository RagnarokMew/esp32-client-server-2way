/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_RECEIVE "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_SEND "edd48740-b4df-4f46-a4e0-267fde365433"

#define DECODED_FINISH "Text has been decoded successfully"

int deviceConnected = 0;
int stage = 0;

int validate(std::string value)
{
  if(value == "RESET")
  {
    stage = 0;
    return 2;
  }

  switch(stage)
  {
    case 0:
      if(value == "STAGE0")
      {
        stage++;
        return 0;
      }
    break;
    case 1:
      if(value == "STAGE1")
      {
        stage++;
        return 0;
      }
    break;
    case 2:
    if(value == "STAGE2")
      {
        stage++;
        return 1;
      }
    break;
    default:
      return 1;
    break;
  }
  return 3;
}

class ServerCallbacks: public BLECharacteristicCallbacks {
  void onConnect(BLEServer* pServer){
    deviceConnected = 1;
  };

  void onDisconect(BLEServer* pServer)
  {
    deviceConnected = 0;
  };

  void onWrite(BLECharacteristic *pCharacteristicReceive)
  {
    std::string value = pCharacteristicReceive->getValue();

    int resultCode = validate(value);

    switch(resultCode)
    {
      case 0:
        Serial.println("Section Correctly Decoded.");
      break;
      case 1:
        Serial.println("Text Correctly Decoded.");
      break;
      case 2:
        Serial.println("Resetting.");
      break;
      default:
        Serial.println("Section Incorrectly Decoded.");
      break;
    }
  }
};

BLECharacteristic *pCharacteristicSend;
BLECharacteristic *pCharacteristicReceive;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Encoded Sender Test");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristicSend = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RECEIVE,
                                         BLECharacteristic::PROPERTY_READ
                                       );

  pCharacteristicReceive = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_SEND,
                                         BLECharacteristic::PROPERTY_WRITE
                                        );

  pCharacteristicReceive->setCallbacks(new ServerCallbacks());
  pCharacteristicSend->setCallbacks(new ServerCallbacks());

  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // put your main code here, to run repeatedly:
    std::string encodedString;
    switch(stage)
    {
      case 0:
        encodedString = "b0894f3766a88397deb9ee5a8d1a9a98";
      break;
      case 1:
        encodedString = "f1af243e2e5d6a670bb80f043a779275";
      break;
      case 2:
        encodedString = "3dc01f5e2765da9ad2b0a2838b28e597";
      break;
      default:
        encodedString = "FULLY DECODED";
      break;
    }
    pCharacteristicSend->setValue(encodedString);
    pCharacteristicSend->notify(false);
    delay(100);
}