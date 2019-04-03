/*
   Based on Neil Kolban and pcbreflux examples for ESP32
   https://github.com/nkolban/esp32-snippets

   ESP32 BLE CTF created by @mjidhage for SecurityFest 2018
*/

// ********************************************************
// ********************************************************
// INCLUDES
#include "sys/time.h"
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"
#include "mbedtls/md.h"
#include "mbedtls/base64.h"
//#include <string>
//#include <base64.h>

// ********************************************************
// ********************************************************
// DEFINES

// THE ONLY CHANGE REQUIRED BETWEEN CLIENTS
#define CLIENTNO 2 //1-4 for clients (1 = no validation, 2 = base64, 3 = md5, 4 = aes256 (key = same as last year))

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep x seconds and then wake up

// All UUIDs required
#define SERVICE_UUID_1_1          "7d4875a8-778f-4bc0-975d-a124b2a21ec4" // UUID 1 128-Bit (may use linux tool uuidgen or random numbers via https://www.uuidgenerator.net/)
#define CHARACTERISTIC_UUID_1_1   "a33dd7a3-0142-4152-8128-783122473e3f"
#define SERVICE_UUID_1_2          "880c2f75-6b46-42dd-b577-1c9dacd479f1" // Client needs this one
#define CHARACTERISTIC_UUID_1_2   "9f4401d4-b7bf-4a53-a3b7-f1f0135b2afd" // Client needs this one

#define SERVICE_UUID_2_1          "588d07f1-0235-4c36-990e-7679e90b205d"
#define CHARACTERISTIC_UUID_2_1   "7291bf8f-a665-40bc-854c-a374729ec5da"
#define SERVICE_UUID_2_2          "c81c1f4d-c1f7-4935-8b1b-2fab17218616" // Client needs this one 
#define CHARACTERISTIC_UUID_2_2   "2d942d77-45f6-4c8a-a492-d904dff96d9b" // Client needs this one

#define SERVICE_UUID_3_1          "06844356-f479-41f2-896c-f759b8749e66"
#define CHARACTERISTIC_UUID_3_1   "86dd4703-23bb-4eeb-95cb-8c139c0c705c"
#define SERVICE_UUID_3_2          "5cd89cce-a2da-4375-b3ce-11dc55d71bc1" // Client needs this one 
#define CHARACTERISTIC_UUID_3_2   "9ce26436-7d13-42d7-ba08-aabc9160df86" // Client needs this one


#define INPUTSIZE 6

// ********************************************************
// ********************************************************
// GLOBALS

RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

BLEAdvertising *pAdvertising;
struct timeval now;
int val = 0;
int noClients = 0;
std::string os;
std::string errorString;
std::string clientName;
std::string deviceName;

// No idea - remove?
#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();
//uint8_t g_phyFuns;

#ifdef __cplusplus
}
#endif

// ********************************************************
// ********************************************************
// CALLBACKS
// Create Characteristic Callbacks for configuration service
class MyCallbackHandler: public BLECharacteristicCallbacks {
    std::string sha_hash = "no data";

    void onRead(BLECharacteristic* pCharacteristic) {
      struct timeval tv;
      gettimeofday(&tv, nullptr);
      std::string buildUp;
      //buildUp += "Time: ";
      //buildUp += tv.tv_sec;
      //buildUp += "Input: ";
      //buildUp += os;
      //buildUp += "Validation: ";
      buildUp += errorString;
      buildUp += " ";
      //buildUp += os;
      pCharacteristic->setValue(buildUp);
    }
    void onWrite(BLECharacteristic* pCharacteristic) {
      // Somebody wrote config to the configuration service
      // Validate the configuration and act on it
      std::string encodedString;
      os = pCharacteristic->getValue();
      Serial.printf("A write-event happened: %s %d\n", os.c_str(), os.length());

      // Assume all is well
      errorString = "ConfigurationOK";

      // VALIDATION - trust, but verify
      switch (CLIENTNO) {
        case 1:
          // VALIDATION None
          // Check length
          if (os.length() > INPUTSIZE) {
            errorString = "ERR:TooLong";
            Serial.print("TooLong");
          }
          if (os.length() < INPUTSIZE) {
            errorString = "ERR:TooShort";
            Serial.print("TooShort");
          }
          break;
        case 2:
          if (os.length() > 100) {
          errorString = "ERR:Overflow";
          Serial.println("Overflow");
          }
          // VALIDATION base64
          // decode base64 - output errors - then check size - same as 1
          char destBuf[250];
          int out;
          int result;
          result = mbedtls_base64_decode((unsigned char *)&destBuf, 120, (size_t *)&out, (const unsigned char *)os.c_str(), strlen(os.c_str()));

          encodedString.assign(destBuf,out);
          
          Serial.println(encodedString.length());
          Serial.println(result);
          Serial.println(out);
          if (result == 0) {
            if (encodedString.length() > INPUTSIZE) {
              errorString = "ERR:TooLong";
              Serial.print("TooLong");
            }
            if (encodedString.length() < INPUTSIZE) {
              errorString = "ERR:TooShort";
              Serial.print("TooShort");
            }

          } else {
            errorString = "ERR:InvalidChar0x002c";
            Serial.print("InvalidChar");
          }
          if (errorString == "ConfigurationOK") {
            os = encodedString;
          }

          break;
        case 3:
          if (os.length() < INPUTSIZE + 65) {
            errorString = "ERR:TooShort";
            Serial.println("TooShort");
          }
          if (os.length() == INPUTSIZE) {
            errorString = "ERR:HashMissing";
            Serial.println("HashMissing");
          }
          if (os.length() > INPUTSIZE + 65) {
            errorString = "ERR:TooLong";
            Serial.println("TooLong");
          }
          if (os.length() == INPUTSIZE + 65) { // Correct size
            if (os.at(INPUTSIZE) != ' ') { // Check delimeter
              errorString = "ERR:WrongDelimeter";
              Serial.println("WrongDelimeter");
            }
            // VALIDATION SHA256
            //char *payload;
            //payload = os.substr(0, INPUTSIZE).c_str();
            byte shaResult[32];
            mbedtls_md_context_t ctx;
            mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
            const size_t payloadLength = strlen(os.substr(0, INPUTSIZE).c_str());
            mbedtls_md_init(&ctx);
            mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
            mbedtls_md_starts(&ctx);
            //mbedtls_md_update(&ctx, (const unsigned char *) payload, payloadLength);
            mbedtls_md_update(&ctx, (const unsigned char *) os.substr(0, INPUTSIZE).c_str(), payloadLength);
            mbedtls_md_finish(&ctx, shaResult);
            mbedtls_md_free(&ctx);
            // Print to monitor
            Serial.print("Hash: ");
            sha_hash = "";
            for (int i = 0; i < sizeof(shaResult); i++) {
              char str[3];
              sprintf(str, "%02x", (int)shaResult[i]);
              Serial.print(str);
              sha_hash += str;
            }
            Serial.println(); // Check hash
            if (os.substr(INPUTSIZE + 1, 64).compare(sha_hash) != 0) {
              errorString = "ERR:HashIsWrong";
              Serial.println("HashisWrong!");
            } else {
              os = os.substr(0, INPUTSIZE); // remove if we add validation in client
            }
          }
          break;
      }


      // No errors detected?
      if (errorString == "ConfigurationOK") {
        clientName = os; // Update name of client - configuration is successful
        Serial.print("Validation OK");
      }
    }
};

// Create Characteristic Callbackhandler for value service
class MyCallbackHandler2: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic* pCharacteristic) {
      std::string readValue;
      //Read and add sensor value
      int val;
      val = hallRead();
      char cstr[16];
      itoa(val, cstr, 10);
      std::string partOne(cstr);
      readValue += partOne;
      readValue += " ";
      //Read and add no of clients connected
      char dstr[16];
      itoa(noClients, dstr, 10);
      std::string partTwo(dstr);
      readValue += partTwo;
      readValue += " ";
      //Add current clientName
      readValue += clientName;
      Serial.printf("Status: %d %d %s\n", val, noClients, clientName.c_str()); //Debug
      pCharacteristic->setValue(readValue);
    }
};

// Create Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      noClients += 1;
      Serial.printf("Another client connected, current count is: %d\n", noClients);
      pAdvertising->start();
    };

    void onDisconnect(BLEServer* pServer) {
      noClients -= 1;
      Serial.printf("A client disconnected, current count is: %d\n", noClients);
    }
};


// ********************************************************
// ********************************************************
// SETUP
void setup() {
  BLEUUID advertServiceUUID;
  BLEUUID advertCharUUID;
  BLEUUID serviceUUID;
  BLEUUID charUUID;
  errorString = "ERR:NoConfiguration";
  Serial.begin(115200);
  gettimeofday(&now, NULL);

  Serial.printf("start ESP32 %d\n", bootcount++);
  Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n", now.tv_sec, now.tv_sec - last);


  last = now.tv_sec;

  switch (CLIENTNO) {
    case 1:
      advertServiceUUID = BLEUUID(SERVICE_UUID_1_2);
      advertCharUUID = BLEUUID(CHARACTERISTIC_UUID_1_2);
      serviceUUID = BLEUUID(SERVICE_UUID_1_1);
      charUUID = BLEUUID(CHARACTERISTIC_UUID_1_1);
      deviceName = "Assured CTF #1";
      break;
    case 2:
      advertServiceUUID = BLEUUID(SERVICE_UUID_2_2);
      advertCharUUID = BLEUUID(CHARACTERISTIC_UUID_2_2);
      serviceUUID = BLEUUID(SERVICE_UUID_2_1);
      charUUID = BLEUUID(CHARACTERISTIC_UUID_2_1);
      deviceName = "Assured CTF #2";
      break;
    case 3:
      advertServiceUUID = BLEUUID(SERVICE_UUID_3_2);
      advertCharUUID = BLEUUID(CHARACTERISTIC_UUID_3_2);
      serviceUUID = BLEUUID(SERVICE_UUID_3_1);
      charUUID = BLEUUID(CHARACTERISTIC_UUID_3_1);
      deviceName = "Assured CTF #3";
      break;
  }


  // ==== DO BLUETOOTH STUFF ====
  // Create the BLE Device
  BLEDevice::init(deviceName);


  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Get the BLEAdvertising object
  pAdvertising = pServer->getAdvertising();
  // Create empty custom AdvertismentData
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  //BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  //oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04
  //oAdvertisementData.setManufacturerData("Advertisment Manu");
  //oAdvertisementData.setName("Advertisment Name");
  oAdvertisementData.setPartialServices(advertServiceUUID);

  // unsigned char md5out = '0123456789123456';
  // const unsigned char strServiceData = '4';
  //*strServiceData = 245;
  // mbedtls_md5(&strServiceData, sizeof(strServiceData), &md5out);
  // Serial.printf("Lite MD5 %s\n",md5out);

  //strServiceData += (char)26;     // Len
  //strServiceData += (char)0xFF;   // Type
  //strServiceData += oBeacon.getData();
  //oAdvertisementData.addData(strServiceData);
  // Set BLEAdvertisingobject data
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setAppearance(0x0d); //Hear rate monitor
  //pAdvertising->setScanResponseData(oScanResponseData);

  // Create the MAGIC SERVICE
  BLEService* pService = pServer->createService(serviceUUID);
  // pAdvertising->addServiceUUID(SERVICE_UUID);

  // Create the characteristic
  BLECharacteristic* pCharacteristic =
    pService->createCharacteristic(charUUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbackHandler());


  // Start the service
  pService->start();

  // Create the sensor service
  BLEService* sService = pServer->createService(advertServiceUUID);
  // pAdvertising->addServiceUUID(SERVICE_UUID);

  // Create the characteristic
  BLECharacteristic* sCharacteristic =
    sService->createCharacteristic(advertCharUUID, BLECharacteristic::PROPERTY_READ);
  sCharacteristic->setCallbacks(new MyCallbackHandler2());
  //Start the service
  sService->start();

  // Start advertising
  std::string logg;
  logg = oAdvertisementData.getPayload();
  Serial.printf("Planning on advertising %s\n", logg.c_str());
  pAdvertising->start();

  Serial.println("Advertizing started...");
  delay(1000000);
  pAdvertising->stop();
  Serial.printf("enter deep sleep\n");
  esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
  Serial.printf("in deep sleep\n");
}

// ********************************************************
// ********************************************************
// LOOP
void loop() {
}
