#include <NimBLEDevice.h> 

// Spark X responses
bool got_message = false;
uint8_t *resp;
int resp_size;

int led_num, red, green, blue;
bool got_lights_message = false;

int tone_bank = 1;

// function defined later
void send_response(uint8_t *resp, int resp_size);

// function in screen.cpp
void change_colour(int led_num, int red, int green, int blue);
void show_tone_bank(int bank);
void show_connected();
void show_disconnected();

uint8_t resp1[]= {0x0b, 0x00, 0x00, 0x00, 0x00, 0x46, 0x34, 0x2e, 0x31, 0x2e, 0x31, 0x39, 0x00};  // firmware F4.1.19
uint8_t resp2[]= {0x0d, 0x00, 0x00, 0x00, 0x03}; // both expression pedals active
uint8_t resp3[]= {0x08, 0x00, 0x00, 0x00, 0x01}; // start with bank 1 active

uint8_t banks[8][13]={{0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x02, 0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x04, 0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75},
                      //{0x14, 0x00, 0x00, 0x00, 0x04, 0x10, 0x11, 0x14, 0x02, 0x03, 0x08, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x05, 0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x06, 0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x07, 0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},                    
                      {0x14, 0x00, 0x00, 0x00, 0x08, 0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75}
                     };

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

void show_addr(uint8_t val[6]) {
  for (int i = 0; i < 6; i++) {
    int v = val[5-i];
    if (v < 16) Serial.print("0");
    Serial.print(v, HEX); 
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
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(": onRead(), value: ");

    int j, l;
    const char *p;
    byte b;
    l = val.length();
    p = val.c_str();
    for (j = 0; j < l; j++) {
      b = p[j];
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    Serial.println();
  };

  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string val = pCharacteristic->getValue();
    Serial.print(">>>> ");
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(": onWrite(), value: ");

    int j, l;
    const char *p;
    byte b;
    l = val.length();
    p = val.c_str();
    for (j = 0; j < l; j++) {
      b = p[j];
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    Serial.println();

    if (p[0] == 0x0b) {
      resp = resp1;
      resp_size = sizeof(resp1);
      got_message = true;
      //send_response(resp, sizeof(resp));
    }

    if (p[0] == 0x0d) {
      resp = resp2;
      resp_size = sizeof(resp2);
      got_message = true;
    }

    if (p[0] == 0x08) {
      resp = resp3;
      resp_size = sizeof(resp3);
      resp[4] = tone_bank;
      got_message = true;
    }

    if (p[0] == 0x14) {
      resp = banks[tone_bank - 1];
      resp_size = sizeof(banks[tone_bank - 1]);
      got_message = true;
    }

    if (p[0] == 0x01) {
      led_num = p[5];
      blue = p[9];
      green = p[10];
      red = p[11];
      if (led_num <= 6)
        got_lights_message = true;
    }
  };
};

static SCCharacteristicCallbacks SCchrCallbacks;

// define here so loop() can see it
BLECharacteristic* pcX2;
BLECharacteristic* pcData1;

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

void SparkControlStart() {
  uint8_t dat[1];
  uint8_t val[1];
  Serial.println("Spark Control emulation");
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
  uint8_t fwrev[]={'F','4','.','1','.','1','9'};    
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

  char scan_data[] = {0x14, 0x09, 'S', 'p', 'a', 'r', 'k', 'X', ' ', 'v', '4', '.', '1', '.', '1', '9', ' ', 'b', 'a', 'e', '3', 
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

  rc = ble_hs_id_gen_rnd(1, &blead);
  if (rc != 0) Serial.println("Rand failed");
  rc = ble_hs_id_set_rnd(blead.val);
  if (rc != 0) Serial.println("Addr failed");
  rc = ble_hs_id_copy_addr(BLE_ADDR_PUBLIC, blead.val, NULL);
  if (rc == 0) show_addr(blead.val);
  rc = ble_hs_id_copy_addr(BLE_ADDR_RANDOM, blead.val, NULL);
  if (rc == 0) show_addr(blead.val);

  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  Serial.println("BLE started");
}

int sparkx_switch = -1, last_sparkx_switch = -1;
int sparkcontrol_switch = 0, last_sparkcontrol_switch = -1;
uint8_t swx_dat[]   =  {0x03, 0x00, 0x00, 0x00, 0x00};
uint8_t sw_dat[]    =  {0x00};
uint8_t slider_dat[] = {0x0c, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x02, 0xff, 0xff};

#define NUM_BUTTONS 8
int spark_control_map[NUM_BUTTONS]  {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00};
//int spark_x_map[2][NUM_BUTTONS]    {{0x00, 0x01, 0x0C, 0xFD, 0x02, 0x03, 0x08, 0xFE},
//                                    {0x10, 0x11, 0x14, 0xFD, 0x12, 0x13, 0x15, 0xFE}};


// this is called from the screen module to send a tone message to the amp or slider values

void send_slider_info(uint16_t slider1_val, uint16_t slider2_val)
{
  slider_dat[5] = slider1_val & 0xff;
  slider_dat[6] = slider1_val >> 8;
  slider_dat[8] = slider2_val & 0xff;
  slider_dat[9] = slider2_val >> 8;
  pcX2->setValue(slider_dat, sizeof(slider_dat));
  pcX2->notify();
}


void send_button_info(int my_btn_num)
{
  // handle Spark X
  int val;
  
  if (my_btn_num == 6)
    val = 0xFD;
  else if (my_btn_num == 7)
    val = 0xFE;
  else
    val = banks[tone_bank - 1][my_btn_num + 5];

  swx_dat[4] = val;
  pcX2->setValue(swx_dat, sizeof(swx_dat));
  pcX2->notify();
  Serial.print("Spark Conrtol X: ");
  Serial.println(val, HEX);
  // swap banks if needed
  if (val == 0xFD) {
    if (tone_bank < 8) tone_bank++;
    Serial.print("Bank changed to: ");
    Serial.println(tone_bank);
    show_tone_bank(tone_bank);
  }
  else if (val == 0xFE) {
    if (tone_bank >1 ) tone_bank--;
    Serial.print("Bank changed to: ");
    Serial.println(tone_bank);
    show_tone_bank(tone_bank);
  };

  // handle Spark Control
  sw_dat[0] = spark_control_map[my_btn_num];
  pcData1->setValue(sw_dat, 1);
  pcData1->notify();
  Serial.print("Spark Conrtol  : ");
  Serial.println(sw_dat[0], HEX);
};

void SparkControlLoop() 
{
  // handle responses to startup messages from amp
  if (got_message) {
    Serial.println("Sending response ");
    pcX2->setValue(resp, resp_size);  
    pcX2->notify();
    got_message = false;
  }

  // handle response to lights messages from amp
  if (got_lights_message) {
    change_colour(led_num, red, green, blue);
    got_lights_message = false;
  }
} 
