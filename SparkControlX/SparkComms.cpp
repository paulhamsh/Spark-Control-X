#include <NimBLEDevice.h> 
#include <Arduino.h>
#include "SparkControlX.h"

// store for a single incoming message
uint8_t message[MESSAGE_SIZE];
int message_len;

QueueHandle_t qFromAmp;

// NimBLE settings 

#define CHAR_READ      NIMBLE_PROPERTY::READ 
#define CHAR_WRITE     NIMBLE_PROPERTY::WRITE
#define CHAR_NOTIFY    NIMBLE_PROPERTY::NOTIFY
#define CHAR_INDICATE  NIMBLE_PROPERTY::INDICATE

// Had to put these here to make the compiler work but not really sure why

BLEService* newService(BLEServer *server, const char *service_UUID, int num_chars);
BLECharacteristic* newCharNoVal(BLEService *pService, const char *char_UUID, uint8_t properties);
BLECharacteristic* newCharData(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t *data, int data_len);
BLECharacteristic* newCharVal(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t val);
BLECharacteristic* newCharVal16(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t val1, uint8_t val2);

// Used by send_data()
BLECharacteristic* pcX2;
BLECharacteristic* pcData1;

void print_hex(uint8_t val) {
    if (val < 16) Serial.print("0");
    Serial.print(val, HEX); 
    Serial.print(" ");
}

void show_addr(char *desc, uint8_t val[6]) {
  Serial.print(desc);
  Serial.print("  ");
  for (int i = 0; i < 6; i++) {
    int v = val[5-i];
    print_hex(v); 
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}

// Server callbacks
class SCServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("Client connected");
    show_connected();
    //BLEDevice::startAdvertising();
  };
  void onDisconnect(BLEServer* pServer) {
    Serial.println("Client disconnected - start advertising");
    show_disconnected();
    BLEDevice::startAdvertising();
  };
};

// Characteristic callbacks
class SCCharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* pCharacteristic){
    std::string val = pCharacteristic->getValue();
    Serial.print("<<<< ");
    //Serial.print(pCharacteristic->getUUID().toString().c_str());
    //Serial.print(": onRead(), value: ");

    int len = val.length();
    const char *data = val.c_str();

    for (int i = 0; i < len; i++) {
      uint8_t v = data[i];
      print_hex(v); 
    }
    Serial.println();
  };

  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string val = pCharacteristic->getValue();
    Serial.print(">>>> ");
    //Serial.print(pCharacteristic->getUUID().toString().c_str());
    //Serial.print(": onWrite(), value: ");

    int len = val.length();
    const char *data = val.c_str();

    for (int i = 0; i < len; i++) {
      uint8_t v = data[i];
      print_hex(v); 
    }
    Serial.println();
    
    struct packet_data qe;
    qe.ptr = (uint8_t *) malloc(len);
    qe.size = len;
    for (int i = 0; i < len; i++)
      qe.ptr[i] = data[i];

    xQueueSend (qFromAmp, &qe, (TickType_t) 0);

    for (int i = 0; i < len; i++) {
      if (i < MESSAGE_SIZE)
        message[i] = data[i];
    }

    message_len = len;
  };
};

static SCCharacteristicCallbacks SCchrCallbacks;


BLEService* newService(BLEServer *server, const char *service_UUID, int num_chars) {
  BLEService *pService;

  pService = server->createService(service_UUID);
  return pService;
}

BLECharacteristic* newCharNoVal(BLEService *pService, const char *char_UUID, uint8_t properties) {
  BLECharacteristic *pChar;

  pChar = pService->createCharacteristic(char_UUID, properties);
  pChar->setCallbacks(&SCchrCallbacks);
  return pChar;
}

BLECharacteristic* newCharData(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t *data, int data_len) {
  BLECharacteristic *pChar;

  pChar = newCharNoVal(pService, char_UUID, properties);
  pChar->setValue(data, data_len); 
  return pChar;
}

BLECharacteristic* newCharVal(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t val) {
  BLECharacteristic *pChar;

  uint8_t dat[1];

  dat[0] = val;
  pChar = newCharNoVal(pService, char_UUID, properties);
  pChar->setValue(dat, 1); 
  return pChar;
}

BLECharacteristic* newCharVal16(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t val1, uint8_t val2) {
  BLECharacteristic *pChar;
  uint8_t dat[2];

  dat[0] = val1;
  dat[1] = val2;
  pChar = newCharNoVal(pService, char_UUID, properties);
  pChar->setValue(dat, 2); 
  return pChar;
}



void spark_comms_start() {
  uint8_t dat[1];
  uint8_t val[1];
  Serial.println("Spark Control emulation");

  qFromAmp = xQueueCreate(20, sizeof (struct packet_data));
  
  static BLEServer* pSCServer;
        
  BLEDevice::init("Spark X"); 
  BLEDevice::setPower(ESP_PWR_LVL_P9); 
  //BLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM);  // Critical for Spark Control and Spark Control X

  pSCServer = BLEDevice::createServer();
  pSCServer->setCallbacks(new SCServerCallbacks());
  // Device Information
  
  BLEService* psDevInf = newService(pSCServer, "180a", 20);  

  uint8_t manuf[]={'A', 'i', 'r', 'T', 'u', 'r', 'n'};
  BLECharacteristic* pcManufName =    newCharData(psDevInf, "2a29", CHAR_READ, manuf, sizeof(manuf)); 
  uint8_t model[]={'S','p','a','r','k','X'};
  BLECharacteristic* pcModelNumber =  newCharData(psDevInf, "2a24", CHAR_READ, model, sizeof(model));
  uint8_t hwrev[]={'H','8','.','0','.','0'};
  BLECharacteristic* pcHwRev =        newCharData(psDevInf, "2a27", CHAR_READ, hwrev, sizeof(hwrev));
  uint8_t fwrev[]={'F','4','.','1','.','2','3'};    
  BLECharacteristic* pcFwRev =        newCharData(psDevInf, "2a26", CHAR_READ, fwrev, sizeof(fwrev));
  uint8_t serial[]={'D','E','7','B','B','0','2','1','4','A','1','8','0','7','6','7'};   
  BLECharacteristic* pcSerialNumber = newCharData(psDevInf, "2a25", CHAR_READ, serial, sizeof(serial));
  uint8_t systemid[] = {0xe3, 0xba, 0x44, 0xc9, 0xff, 0x0f, 0x62, 0x88};
  BLECharacteristic* pcSystemID =     newCharData(psDevInf, "2a23", CHAR_READ, systemid, sizeof(systemid));
  uint8_t pnpid[] = {0x01, 0x22, 0x01, 0x12, 0x00, 0x01, 0x00};
  BLECharacteristic* pcPnPID =        newCharData(psDevInf, "2a50", CHAR_READ, pnpid, sizeof(pnpid));
  
  // Battery Service
  BLEService* psBat = newService(pSCServer, "180f", 10);
  BLECharacteristic* pcBatLvl = newCharVal(psBat, "2a19", CHAR_READ | CHAR_NOTIFY, 80);

  // Service A
  BLEService* psA = newService(pSCServer, "34452F38-9E44-46AB-B171-0CC578FEB928", 20);
  uint8_t A1[14] = {0x04, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x83, 0x00};    
  BLECharacteristic* pcA1 = newCharData(psA, "CAD0C949-7DCE-4A04-9D80-E767C796B392", CHAR_READ, A1, 14);
  BLECharacteristic* pcA2 = newCharVal(psA, "76E7DF30-E3AD-41B1-A05A-279C80FC7FB4", CHAR_READ, 0x0f);
  BLECharacteristic* pcA3 = newCharVal(psA, "CDB3A16B-02E1-4BC0-843C-665CBEB378B7", CHAR_READ, 0x01);
  BLECharacteristic* pcA4 = newCharVal16(psA, "53EEA35F-27F1-46E4-A790-34BECD28B701", CHAR_READ | CHAR_NOTIFY, 0x00, 0x01);
  BLECharacteristic* pcA5 = newCharVal(psA, "54eea35f-27f1-46e4-a790-34becd28b701", CHAR_READ | CHAR_NOTIFY, 0x00);

  // B Service
  BLEService* psB = newService(pSCServer, "6FACFE71-A4C3-4E80-BA5C-533928830727", 10);
  BLECharacteristic* pcB1 = newCharVal(psB, "90D9A098-9CD8-4A7A-B176-91FFE80909F2", CHAR_READ | CHAR_NOTIFY, 0x00);

  // C Service
  BLEService* psC = newService(pSCServer, "5cb68410-6774-11e4-9803-0800200c9a66", 10);
  BLECharacteristic* pcC1 = newCharNoVal(psC, "407eda40-6774-11e4-9803-0800200c9a66", CHAR_WRITE);

  // Data Service
  BLEService* psData = newService(pSCServer, "7bdb8dc0-6c95-11e3-981f-0800200c9a66", 10);
  pcData1 = newCharVal(psData, "362f71a0-6c96-11e3-981f-0800200c9a66", CHAR_READ | CHAR_NOTIFY, 0x00);
  uint8_t datData2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  BLECharacteristic* pcData2 = newCharData(psData, "BD066DA4-F9EC-4F0D-A53C-1CD99147A641", CHAR_READ | CHAR_NOTIFY, datData2, 16);
  BLECharacteristic* pcData3 = newCharVal(psData, "85da4f4b-a2ca-4c7c-8c7d-fcd9e2daad56", CHAR_READ | CHAR_NOTIFY, 0x01);

  // E Service
  BLEService* psE = newService(pSCServer, "FE59", 10);
  BLECharacteristic* pcE1 = newCharNoVal(psE, "8ec90003-f315-4f60-9fb8-838830daea50", CHAR_WRITE | CHAR_INDICATE);
 
  // F Service        
  BLEService* psF = newService(pSCServer, "25a22330-820f-11e3-baa7-0800200c9a66", 10);
  uint8_t datF1[12] = {0x32, 0x2c, 0x73, 0x31, 0x33, 0x32, 0x5f, 0x37, 0x2e, 0x32, 0x2e, 0x30};
  BLECharacteristic* pcF1 = newCharData(psF, "ba7cc552-cc2c-404e-bf75-8778f023787d", CHAR_READ, datF1, 12);
      
  // G Service            
  BLEService* psG = newService(pSCServer, "03b80e5a-ede8-4b33-a751-6ce34ec4c700", 10);
  BLECharacteristic* pcG1 = newCharNoVal(psG, "7772e5db-3868-4112-a1a9-f2669d106bf3", CHAR_READ | CHAR_WRITE | CHAR_NOTIFY);

  // H Service
  BLEService* psH = newService(pSCServer, "97a16290-8c08-11e3-baa8-0800200c9a66", 40);
  BLECharacteristic* pcH1 = newCharVal(psH, "640c0d80-9b4f-11e3-a5e2-0800200c9a66", CHAR_READ | CHAR_WRITE, 0x00);
  BLECharacteristic* pcH2 = newCharVal(psH, "49aa8950-a40d-11e3-a5e2-0800200c9a66", CHAR_READ | CHAR_WRITE, 0x00);
  BLECharacteristic* pcH3 = newCharNoVal(psH, "3673862E-780A-4936-9C56-5477A6C62B94", CHAR_READ);
  uint8_t datH4[19] = {0x05, 0x03, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
  BLECharacteristic* pcH4 = newCharData(psH, "fbeeeb29-c82a-4fb6-a5f7-aee512c93790", CHAR_READ | CHAR_WRITE, datH4, 19);
  BLECharacteristic* pcH5 = newCharVal(psH, "75cbbe2f-09ec-4b34-86c4-6124c4dd68f4", CHAR_READ, 0x28);
  BLECharacteristic* pcH6 = newCharNoVal(psH, "1bcf02e0-d53e-468c-9d6d-e9251230d8c9", CHAR_READ | CHAR_WRITE);
  BLECharacteristic* pcH7 = newCharVal16(psH, "7b51af7d-f28a-48be-8081-6a647e6249eb", CHAR_READ, 0x20, 0x1c);
  BLECharacteristic* pcH8 = newCharVal(psH, "db3207d4-97ff-4497-ad1d-0ba1bccc56ba", CHAR_READ | CHAR_WRITE, 0x01);
  BLECharacteristic* pcH9 = newCharVal(psH, "b8980e72-799c-4172-af97-46380afb068c", CHAR_READ | CHAR_WRITE, 0x01);
  BLECharacteristic* pcHA = newCharVal16(psH, "13252451-fc8e-46e7-8dce-c799276bd61c", CHAR_READ | CHAR_WRITE, 0x32, 0x00);
  BLECharacteristic* pcHB = newCharNoVal(psH, "29eaf09d-996d-4d94-a989-ed320fbac5c9", CHAR_WRITE);
  BLECharacteristic* pcHC = newCharNoVal(psH, "733ee9cc-174d-452E-b45c-0402aa75ff75", CHAR_READ | CHAR_WRITE);

  
  // Spark services
  BLEService* psS = newService(pSCServer, "ffc0", 40);
  BLECharacteristic* pcS1 = newCharNoVal(psS, "ffc1", CHAR_WRITE);
  BLECharacteristic* pcS2 = newCharNoVal(psS, "ffc2", CHAR_READ | CHAR_NOTIFY);
  
  // Spark X services
  BLEService* psX = newService(pSCServer, "ffc8", 40);
  BLECharacteristic* pcX1 = newCharNoVal(psX, "ffc9", CHAR_WRITE);
  pcX2 = newCharNoVal(psX, "ffca", CHAR_READ | CHAR_NOTIFY);

  BLEService* psN = newService(pSCServer, "8d53dc1d-1db7-4cd3-868b-8a527460aa84", 40);
  BLECharacteristic* pcN1 = newCharNoVal(psN, "da2e7828-fbce-4e01-se9e-261174997c48", CHAR_WRITE | CHAR_NOTIFY);

  
  psDevInf->start();
  psBat->start();
  psA->start();
  psB->start();
  psC->start();    
  psData->start();
  psE->start(); 
  psF->start();
  psG->start();
  psH->start();
  psS->start();
  psX->start();
  psN->start();
 
  /*
  psDevInf->dump();
  psBat->dump();
  psA->dump();
  psB->dump();
  psC->dump();    
  psData->dump();
  psE->dump(); 
  psF->dump();
  psG->dump();
  psH->dump();
  //psS->dump();
  //psX->dump();
*/

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();

  char scan_data[] = {0x14, 0x09, 'S', 'p', 'a', 'r', 'k', 'X', ' ', 'v', '4', '.', '1', '.', '2', '3', ' ', 'b', 'a', 'e', '3', 
                      //0x53, 0x70, 0x61, 0x72, 0x6b, 0x58, 0x20, 0x76, 0x34, 0x2e, 0x31, 0x2e, 0x31, 0x39, 0x20, 0x62, 0x61, 0x65, 0x33, 
                      0x03, 0x19, 0xc1, 0x03};
  char adv_data[] =  {0x02, 0x01, 0x06, 
                      0x05, 0xff, 0x22, 0x01, 0x00, 0x12, 
                      0x11, 0x07, 0x00, 0xc7, 0xc4, 0x4e, 0xe3, 0x6c, 0x51, 0xa7, 0x33, 0x4b, 0xe8, 0xed, 0x5a, 0x0e, 0xb8, 0x03};

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanAdvertisementData = BLEAdvertisementData();  

  oScanAdvertisementData.addData(scan_data, sizeof(scan_data));
  oAdvertisementData.addData(adv_data, sizeof(adv_data));

  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanAdvertisementData);

  ble_addr_t blead;
  int rc;

  BLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM, false);
  //BLEDevice::setOwnAddrTypeRandom();

  rc = ble_hs_id_gen_rnd(1, &blead);
  if (rc != 0) Serial.println("Rand failed");

  rc = ble_hs_id_set_rnd(blead.val);
  if (rc != 0) Serial.println("Addr failed");

  rc = ble_hs_id_copy_addr(BLE_ADDR_PUBLIC, blead.val, NULL);
  if (rc == 0) show_addr("PUBLIC", blead.val);
  rc = ble_hs_id_copy_addr(BLE_ADDR_RANDOM, blead.val, NULL);
  if (rc == 0) show_addr("RANDOM", blead.val);


  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  Serial.println("BLE started");
}

int get_message(uint8_t *buf)
{
  int len = message_len;
  if (len > MESSAGE_SIZE) 
    len = MESSAGE_SIZE;
  memcpy(buf, message, len);
  message_len = 0;
  return len;
}

void send_spark_x_data(uint8_t *buf, int len) {
  pcX2->setValue(buf, len);
  pcX2->notify();

  Serial.print("Sending: ");
  for (int i = 0; i < len; i++) {
    byte v = buf[i];
    print_hex(v); 
    }
  Serial.println();
}

void send_spark_control_data(uint8_t *buf, int len) {
  pcData1->setValue(buf, len);
  pcData1->notify();
}
