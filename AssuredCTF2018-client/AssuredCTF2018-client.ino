/**
 * A BLE client example that is rich in capabilities.
 */

#include "BLEDevice.h"
//#include "BLEScan.h"

#define SERVICE_UUID_2_2          "c81c1f4d-c1f7-4935-8b1b-2fab17218616" // Client needs this one 
#define CHARACTERISTIC_UUID_2_2   "2d942d77-45f6-4c8a-a492-d904dff96d9b" // Client needs this one
#define SERVICE_UUID_1_2          "880c2f75-6b46-42dd-b577-1c9dacd479f1" // Client needs this one
#define CHARACTERISTIC_UUID_1_2   "9f4401d4-b7bf-4a53-a3b7-f1f0135b2afd" // Client needs this one
#define SERVICE_UUID_3_2          "5cd89cce-a2da-4375-b3ce-11dc55d71bc1" // Client needs this one 
#define CHARACTERISTIC_UUID_3_2   "9ce26436-7d13-42d7-ba08-aabc9160df86" // Client needs this one

// The remote service we wish to connect to.
static BLEUUID serviceUUID1(SERVICE_UUID_1_2);
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID1(CHARACTERISTIC_UUID_1_2);

// The remote service we wish to connect to.
static BLEUUID serviceUUID2(SERVICE_UUID_2_2);
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID2(CHARACTERISTIC_UUID_2_2);

// The remote service we wish to connect to.
static BLEUUID serviceUUID3(SERVICE_UUID_3_2);
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID3(CHARACTERISTIC_UUID_3_2);

static BLEAddress *pServerAddress1;
static BLEAddress *pServerAddress2;
static BLEAddress *pServerAddress3;

static boolean doConnect1 = false;
static boolean doConnect2 = false;
static boolean doConnect3 = false;

static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic1;
static BLERemoteCharacteristic* pRemoteCharacteristic2;
static BLERemoteCharacteristic* pRemoteCharacteristic3;


static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    //Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    //Serial.print(" of data length ");
    //Serial.println(length);
}

// Using the BLEAdress, connect to server 1 and print the value
bool connectToServer1(BLEAddress pAddress) {
    //Serial.print("Forming a connection to ");
    //Serial.println(pAddress.toString().c_str());   
    BLEClient*  pClient1  = BLEDevice::createClient();
    //Serial.println(" - Created client");
    // Connect to the remove BLE Server.
    if (!pClient1->connect(pAddress)) {
      Serial.print("Connection fail!");
      
      pClient1->~BLEClient();
      pClient1->disconnect();
      
      return false;
    }
    //Serial.println(" - Connected to server");
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService1 = pClient1->getService(serviceUUID1);
    if (pRemoteService1 == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID1.toString().c_str());
      
      pClient1->~BLEClient();
      pClient1->disconnect();
      
      return false;
    }
    //Serial.println(" - Found our service");
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic1 = pRemoteService1->getCharacteristic(charUUID1);
    if (pRemoteCharacteristic1 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID1.toString().c_str());
      
      pClient1->~BLEClient();
      pClient1->disconnect();
      
      return false;
    }
    //Serial.println(" - Found our characteristic");
    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic1->readValue();
    Serial.print("                                                                                     S1: ");
    Serial.println(value.c_str());
    pRemoteCharacteristic1->registerForNotify(notifyCallback);
    
    pClient1->~BLEClient();
    pClient1->disconnect();
    
    return true;
}

bool connectToServer2(BLEAddress pAddress) {
    //Serial.print("Forming a connection to ");
    //Serial.println(pAddress.toString().c_str());   
    BLEClient*  pClient2  = BLEDevice::createClient();
    //Serial.println(" - Created client");
    // Connect to the remove BLE Server.
    if (!pClient2->connect(pAddress)) {
      Serial.print("Connection fail!");
      pClient2->disconnect();
      pClient2->~BLEClient();
      return false;
    }    //Serial.println(" - Connected to server");
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService2 = pClient2->getService(serviceUUID2);
    if (pRemoteService2 == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID2.toString().c_str());
      pClient2->disconnect();
      pClient2->~BLEClient();
      return false;
    }
    //Serial.println(" - Found our service");
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic2 = pRemoteService2->getCharacteristic(charUUID2);
    if (pRemoteCharacteristic2 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID2.toString().c_str());
      pClient2->disconnect();
      pClient2->~BLEClient();
      return false;
    }
    //Serial.println(" - Found our characteristic");
    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic2->readValue();
    Serial.print("                                                                                     S2: ");
    Serial.println(value.c_str());
    pRemoteCharacteristic2->registerForNotify(notifyCallback);
    pClient2->disconnect();
    pClient2->~BLEClient();
    return true;
}

bool connectToServer3(BLEAddress pAddress) {
    //Serial.print("Forming a connection to ");
    //Serial.println(pAddress.toString().c_str());   
    BLEClient*  pClient3  = BLEDevice::createClient();
    //Serial.println(" - Created client");
    // Connect to the remove BLE Server.
    if (!pClient3->connect(pAddress)) {
      Serial.print("Connection fail!");
      pClient3->disconnect();
      pClient3->~BLEClient();
      return false;
    }
    //Serial.println(" - Connected to server");
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService3 = pClient3->getService(serviceUUID3);
    if (pRemoteService3 == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID3.toString().c_str());
      pClient3->disconnect();
      pClient3->~BLEClient();
      return false;
    }
    //Serial.println(" - Found our service");
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic3 = pRemoteService3->getCharacteristic(charUUID3);
    if (pRemoteCharacteristic3 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID3.toString().c_str());
      pClient3->disconnect();
      pClient3->~BLEClient();
      return false;
    }
    //Serial.println(" - Found our characteristic");
    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic3->readValue();
    Serial.print("                                                                                     S3: ");
    Serial.println(value.c_str());
    pRemoteCharacteristic3->registerForNotify(notifyCallback);
    pClient3->disconnect();
    pClient3->~BLEClient();
    return true;
}


/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  //Called for each advertising BLE server.
  void onResult(BLEAdvertisedDevice advertisedDevice) {
   // Serial.print("BLE Advertised Device found: ");
   //  Serial.print(advertisedDevice.haveServiceUUID());
   // Serial.print(" ");
   // Serial.print(advertisedDevice.toString().c_str());
    
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID()) {
      if (advertisedDevice.getServiceUUID().equals(serviceUUID1)) { // Found server 1
        Serial.println(" Found server 1 "); 
        // advertisedDevice.getScan()->stop();
        pServerAddress1 = new BLEAddress(advertisedDevice.getAddress());
        doConnect1 = true;
      } else {
        if (advertisedDevice.getServiceUUID().equals(serviceUUID2)) { // Found server 2
          Serial.println(" Found server 2 "); 
          // advertisedDevice.getScan()->stop();
          pServerAddress2 = new BLEAddress(advertisedDevice.getAddress());
          doConnect2 = true;
        } else {
            if (advertisedDevice.getServiceUUID().equals(serviceUUID3)) { // Found server 3
            Serial.println(" Found server 3 "); 
            // advertisedDevice.getScan()->stop();
            pServerAddress3 = new BLEAddress(advertisedDevice.getAddress());
            doConnect3 = true;
          } else {
          //  Serial.println(" Wrong UUID! ");
          }
        }
      }
    } else {
     // Serial.println(" No services! ");
    }
  } // onResult
}; // MyAdvertisedDeviceCallbacks

void doNewScan() {
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5);
}


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  doNewScan();
  
} // End of setup.


// This is the Arduino main loop function.
void loop() {
  if (!doConnect1 || !doConnect2 || !doConnect3) { // At least one server is not connected - scan again.
    doNewScan();
  }

  // If the flag "doConnect" is true then we have scanned for and found a desired
  // BLE Server with which we wish to connect.  Now we connect to it and read a value.
  if (doConnect1 == true) {
    if (connectToServer1(*pServerAddress1)) {
    // Serial.println("We are now connected to BLE Server 1.");
    // connected = true;
    } else {
      Serial.println("We have failed to connect to server 1");
      doConnect1 = false;
    }
    
    //connected = false;
  }

  if (doConnect2 == true) {
    if (connectToServer2(*pServerAddress2)) {
     // Serial.println("We are now connected to BLE Server 2.");
     // connected = true;
    } else {
      Serial.println("We have failed to connect to server 2");
      doConnect2 = false;
    }
    
    //connected = false;
  }

  if (doConnect3 == true) {
      if (connectToServer3(*pServerAddress3)) {
       // Serial.println("We are now connected to BLE Server 3.");
       // connected = true;
      } else {
        Serial.println("We have failed to connect to server 3");
        doConnect3 = false;
      }
      
      //connected = false;
    }
  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  //if (connected) {
    //String newValue = "Time since boot: " + String(millis()/1000);
    //Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    
    // Set the characteristic's value to be the array of bytes that is actually a string.
    //pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
    //std::string value = pRemoteCharacteristic->readValue();
    //Serial.print("The characteristic value was: ");
    //Serial.println(value.c_str());
  //}
  
  delay(5000); // Delay 5 seconds between loops.
} // End of loop
