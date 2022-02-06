/**
   USE OF THIS SOFTWARE IS GOVERNED BY THE TERMS AND CONDITIONS
   OF THE LICENSE STATEMENT AND LIMITED WARRANTY FURNISHED WITH
   THE PRODUCT.
   <p/>
   IN PARTICULAR, YOU WILL INDEMNIFY AND HOLD B2N LTD., ITS
   RELATED COMPANIES AND ITS SUPPLIERS, HARMLESS FROM AND AGAINST ANY
   CLAIMS OR LIABILITIES ARISING OUT OF THE USE, REPRODUCTION, OR
   DISTRIBUTION OF YOUR PROGRAMS, INCLUDING ANY CLAIMS OR LIABILITIES
   ARISING OUT OF OR RESULTING FROM THE USE, MODIFICATION, OR
   DISTRIBUTION OF PROGRAMS OR FILES CREATED FROM, BASED ON, AND/OR
   DERIVED FROM THIS SOURCE CODE FILE.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BLUETOOTH/BLE DISPLAY

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ble constants
#define BLE_DEFAULT_MTU 500
// in msec
#define BLE_SHARE_SECRET_AFTER_RESTART_TIMEOUT 300000
#define BLE_NAME_PREFIX "BV("
#define BLE_MAX_SEQUNECE 512

#include "GatewayBleCommand.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// additional optional logs
//
//static void my_gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t* param)
//{
////  LOG64_SET(F("BLE: Custom gattc event handler, event:"));
////  LOG64_SET((uint32_t)event);
////  LOG64_NEW_LINE;
//
//  ESP_LOGI(LOG_TAG, "custom gattc event handler, event: %d", (uint8_t)event);
//}
//
//static void my_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gatts_cb_param_t* param)
//{
//  ESP_LOGI(LOG_TAG, "custom gatts event handler, event: %d", (uint8_t)event);
//
////  LOG64_SET(F("BLE: Custom gattc event handler, event:"));
////  LOG64_SET((uint32_t)event);
////  LOG64_NEW_LINE;
//}
//
//static void my_gap_event_handler(esp_gap_ble_cb_event_t  event, esp_ble_gap_cb_param_t* param)
//{
//  ESP_LOGI(LOG_TAG, "custom gap event handler, event: %d", (uint8_t)event);
//
////  LOG64_SET(F("BLE: Custom gap event handler, event:"));
////  LOG64_SET((uint32_t)event);
////  LOG64_NEW_LINE;
//}

#define BLE_TIMEOUT_RUN 1
uint32_t bleLastRun;

#define BLE_TRANSPORT_CLEANUP_TIMEOUT 5000
#define BLE_CONNECTION_TIMEOUT 1000
#define BLE_DISCONNECT_TIMEOUT 1800000
#define BLE_READVERTIZING_TIMEOUT 1800000

BLEServer* bleServer;
BLECharacteristic* bleCharacteristicNonInteractive;
BLECharacteristic* bleCharacteristicInTransport;
BLECharacteristic* bleCharacteristicOutTransport;

bool bleConnected;
bool bleOldConnected;
uint32_t bleConnectionLast;
uint32_t bleConnectedLast;
uint32_t bleAdvertizingLast;

#define BLE_SERVICE_UUID                        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHARACTERISTIC_TRANSPORT_IN_UUID    "beb5483e-36e1-4689-b7f5-ea07361b26a8"
#define BLE_CHARACTERISTIC_TRANSPORT_OUT_UUID   "beb5483e-36e1-4690-b7f5-ea07361b26a8"


#define BLE_MAX_BUF_SIZE 640
#define BLE_MAX_BUF_SIZE_WITH_SEQ (11 + BLE_MAX_BUF_SIZE)
#define BLE_LOG_MAX_BUF_SIZE BLE_MAX_BUF_SIZE
#define BLE_LOG_DEFAULT_BUF_SIZE 256
uint16_t bleLogBufSize = BLE_LOG_DEFAULT_BUF_SIZE;
char bleLogBuf[11 + BLE_LOG_MAX_BUF_SIZE];
uint16_t bleLogSize;
uint32_t bleLogSeq;


char bleTransportResponseBuf[BLE_MAX_BUF_SIZE_WITH_SEQ];
const char * bleTransportResponse; // used as flag for data ready to process
uint32_t bleTransportResponseSet;

char bleTransportRequestBuf[BLE_MAX_BUF_SIZE_WITH_SEQ];
char * bleTransportRequest; // used as a flag for data ready to process


class bleServiceCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param)
    {
      //      LOG64_SET("BLE: CONNECTED CALLBACK ENTRY");
      //      LOG64_NEW_LINE;

      pServer->updateConnParams(param->connect.remote_bda, 0x06, 0x12, 0, 100);

      //      LOG64_SET("BLE: CONNECTED PARAMS UPDATED");
      //      LOG64_NEW_LINE

      bleLogBufSize = BLE_LOG_DEFAULT_BUF_SIZE;

      bleTransportResponse = NULL;
      bleTransportRequest = NULL;

      if ((bleConnectedLast = millis()) == 0 )
      {
        bleConnectedLast = 1;
      }

      bleAdvertizingLast = 0 ;

      bleConnected = true;

      //      LOG64_SET("BLE: CONNECTED CALLBACK EXIT");
      //      LOG64_NEW_LINE


    };

    void onDisconnect(BLEServer* pServer)
    {
      bleServer->startAdvertising(); // restart advertising
      if ((bleAdvertizingLast = millis()) == 0 )
      {
        bleAdvertizingLast = 1;
      }

      bleLogBufSize = BLE_LOG_DEFAULT_BUF_SIZE;

      bleTransportResponse = NULL;
      bleTransportRequest = NULL;

      bleConnectedLast = 0;

      bleConnected = false;



    }
};


inline void blePrepareTransportResponse(const char * seq, const char * & bleTransportResponsePtr)
{
  size_t seqLen = strlen(seq);
  size_t delimiterSize = BLE_DELIMITER.length();
  size_t dataSize = strlen(bleTransportResponseBuf);
  memmove(&bleTransportResponseBuf[seqLen + delimiterSize], bleTransportResponseBuf, dataSize + 1);
  memcpy(bleTransportResponseBuf, seq, seqLen);
  memcpy(&bleTransportResponseBuf[seqLen], BLE_DELIMITER.c_str(), delimiterSize);

  bleTransportResponsePtr = bleTransportResponseBuf;

  bleTransportResponseSet = millis();
}

//bleTransportRequestPtr used as a flag ready to process
//bleTransportResponsePtr used as a flag ready to response
//ret < 0 means wrong message reject , ret >= 0 means OK (0 - no log response, 1 log response)
int8_t bleProcessRequest(char * & bleTransportRequestPtr, const char * & bleTransportResponsePtr)
{
  int8_t ret = -1;
  if (bleTransportRequestPtr != NULL)
  {
    size_t requestLen = strlen(bleTransportRequestPtr);
    if (requestLen > 0)
    {
      if (requestLen > BLE_DEFAULT_MTU)
      {
        LOG64_SET(F("BLE: TRANSPORT PACKET TOO LONG["));
        LOG64_SET(requestLen);
        LOG64_SET(F("]"));
        LOG64_NEW_LINE;
      }
      else
      {
        ret = 1;

        char * stringSeq =  strtok (bleTransportRequestPtr, BLE_DELIMITER.c_str());
        char * stringOper =  strtok (NULL, BLE_DELIMITER.c_str());

        if (stringOper != NULL)
        {
          uint16_t oper = atoi(stringOper);

          ////////////////////////////////////////////
          //  security operation - request secret and compare with user provided to enable device settings : Secret @ hash
          if (oper == BLE_SECRET_GET_OPERATION)
          {
            // generate secret
            String secret_str = String(GetSetupSecret());
            char sha_bin[21];
            SHA1(sha_bin, secret_str.c_str(), secret_str.length());
            String sha_hex = String();
            for (int i = 0; i < 5; i++)
            {
              sha_hex += String(sha_bin[i], HEX);
            }
            if (millis() < BLE_SHARE_SECRET_AFTER_RESTART_TIMEOUT)
            {
              // in case we are less then 5 minutes before start
              sha_hex += BLE_ELEMENT_END;
            }
            // fill response
            strcpy(bleTransportResponseBuf, sha_hex.c_str());

            blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);

            ret = 0;
          }
          ////////////////////////////////////////////
          //  factory set
          else if (oper >= BLE_FACTORY_SET_OPERATION) // please place bigger in front
          {
            // set setting
            oper -= BLE_FACTORY_SET_OPERATION;
            if (oper < BLE_FACTORY_OPERATION_COUNT)
            {
              char * stringValue =  strtok (NULL, BLE_DELIMITER.c_str());
              if (stringValue != NULL)
              {
                bleSetFactory(oper, stringValue);
              }
              bleTransportResponseBuf[0] = 0;
              blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);
            }
          }
          ////////////////////////////////////////////
          //  factory get - field name @ value
          else if (oper >= BLE_FACTORY_GET_OPERATION) // please place bigger in front
          {
            // request for settings value
            oper -= BLE_FACTORY_GET_OPERATION;
            if (oper < BLE_FACTORY_OPERATION_COUNT)
            {
              if (bleGetFactory(oper, bleTransportResponseBuf))
              {
                blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);
              }
            }
          }
          ////////////////////////////////////////////
          //  set OTA operation - settings id @ value
          else if (oper >= BLE_OTA_SET_OPERATION) // please place bigger in front
          {
            // set setting
            oper -= BLE_OTA_SET_OPERATION;
            if (oper < BLE_OTA_OPERATION_COUNT)
            {
              // custom handling OTAWrite
              if (oper == 1)
              {
                size_t stringOperLen = strlen(stringOper);
                char * startPosition = stringOper + stringOperLen + 1;
                // revert the magic of strtok :) - start from oper as we will use the seq in the response
                for (char * i = startPosition; i < (bleTransportRequestPtr + stringOperLen); i++)
                {
                  if (i[0] == 0)
                  {
                    i[0] = BLE_DELIMITER.c_str()[0];
                  }
                }

                bleSetOta(oper, startPosition, bleTransportResponseBuf);
              }
              else
              {
                char * stringValue =  strtok (NULL, BLE_DELIMITER.c_str());
                // NULL supported for some operations

                bleSetOta(oper, stringValue, bleTransportResponseBuf);
              }

              blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);
            }

            ret = 0;
          }
          ////////////////////////////////////////////
          //  get OTA field value - field name @ value
          else if (oper >= BLE_OTA_GET_OPERATION) // please place bigger in front
          {
            // request for settings value
            oper -= BLE_OTA_GET_OPERATION;
            if (oper < BLE_OTA_FIELDS_INDEX_COUNT)
            {
              if (bleGetOta(oper, bleTransportResponseBuf))
              {
                blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);
              }
            }
          }
          ////////////////////////////////////////////
          //  get BLE ( not app revuision - for app revision you need OTA revision) revision operation
          else if (oper == BLE_REVISION_GET_OPERATION) // please place bigger in front
          {
            size_t secretFieldNameSize = strlen(BLE_REVISION_DATA_FIELD);
            size_t delimiterSize = BLE_DELIMITER.length();

            strcpy(bleTransportResponseBuf, BLE_REVISION_DATA_FIELD);
            strcpy(&bleTransportResponseBuf[secretFieldNameSize], BLE_DELIMITER.c_str());
            strcpy(&bleTransportResponseBuf[secretFieldNameSize + delimiterSize], BLE_REVISION);

            blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);
          }
          ////////////////////////////////////////////
          //  get log operation - get log line
          else if (oper == BLE_LOG_GET_OPERATION) // please place bigger in front
          {
            // schedule LOG
            bleLogBuf[bleLogSize++] = 0;

            strcpy(bleTransportResponseBuf, bleLogBuf);

            bleLogSize = 0;

            blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);

            ret =  0;
          }
          ////////////////////////////////////////////
          //  set setting operation - settings id @ value
          else if (oper >= BLE_SETTING_SET_OPERATION) // please place bigger in front
          {
            // set setting
            oper -= BLE_SETTING_SET_OPERATION;
            if (oper < BLE_SETTING_COUNT)
            {
              char * stringValue =  strtok (NULL, BLE_DELIMITER.c_str());
              if (stringValue == NULL)
              {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
                stringValue = "";
#pragma GCC diagnostic pop
              }

              bleSetSetting(oper, stringValue, bleTransportResponseBuf);

              blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);
            }
          }
          ////////////////////////////////////////////
          //  get setting operation - settings name @ value
          else if (oper >= BLE_SETTING_GET_OPERATION) // please place bigger in front
          {
            // request for settings value
            oper -= BLE_SETTING_GET_OPERATION;
            if (oper < BLE_SETTING_COUNT)
            {
              if (bleGetSetting(oper, bleTransportResponseBuf))
              {
                blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);
              }
            }
          }
          ////////////////////////////////////////////
          //  get all settings option operation - group, fields, type whne no more # will be returned
          else if (oper >= BLE_SETTING_GET_DESC_OPERATION) // please place bigger in front
          {
            // request for settings description
            oper -= BLE_SETTING_GET_DESC_OPERATION;
            if (oper < BLE_SETTING_DESC_COUNT)
            {
              strcpy(bleTransportResponseBuf, BLE_SETTING_DESC[oper]);
            }
            else
            {
              strcpy(bleTransportResponseBuf, BLE_ELEMENT_END.c_str());
            }

            blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);
          }
          ////////////////////////////////////////////
          //   get all dada fields option operation - group, fields, type whne no more # will be returned
          else if (oper >= BLE_DATA_FIELDS_BASE_GET_OPERATION) // please place bigger in front
          {

            // request for datafield name
            oper -= BLE_DATA_FIELDS_BASE_GET_OPERATION;
            if (oper < BLE_DATA_FIELDS_INDEX_COUNT)
            {
              strcpy(bleTransportResponseBuf, BLE_DATA_FIELDS[oper]);
            }
            else
            {
              strcpy(bleTransportResponseBuf, BLE_ELEMENT_END.c_str());
            }

            blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);

          }
          ////////////////////////////////////////////
          //return value for requested field - name @ value #
          else if (oper < BLE_DATA_FIELDS_INDEX_COUNT)
          {
            if (bleGetCommand(oper, bleTransportResponseBuf))
            {
              blePrepareTransportResponse(stringSeq, bleTransportResponsePtr);
            }
          }
        }
      }
    }

    bleTransportRequestPtr = NULL;
  }

  return ret;
}


void bleProcessResponse(uint8_t doLogResponse, const char * & bleTransportResponsePtr)
{

  if (bleTransportResponsePtr != NULL)
  {
    bleCharacteristicOutTransport->setValue(bleTransportResponsePtr);

    switch (doLogResponse)
    {
      case 0 : // do nothings
        break;
      case 1:
        {
          LOG64_SET(F("BLE: TRANSPORT SEND["));
          LOG64_SET(bleTransportResponsePtr);
          LOG64_SET(F("]"));
          LOG64_NEW_LINE;
        }
        break;
    }
    bleTransportResponsePtr = NULL;

    bleCharacteristicOutTransport->notify();
  }


}

//int tt = 1;

class bleCharacteristicInTransportCallback: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {

      // reset disconnect monitor cycle
      if ((bleConnectedLast = millis()) == 0 )
      {
        bleConnectedLast = 1;
      }

      uint16_t mtuMaxPacket = bleServer->getPeerMTU(bleServer->getConnId()) - 30;
      if (mtuMaxPacket > BLE_LOG_MAX_BUF_SIZE)
      {
        mtuMaxPacket = BLE_LOG_MAX_BUF_SIZE;
      }

      if (mtuMaxPacket != bleLogBufSize)
      {
        bleLogBufSize = mtuMaxPacket;
      }

      if (bleTransportRequest == NULL)
      {

        std::string stringValue = pCharacteristic->getValue();
        //
        //                        Serial.print("BLE: RECIEVED[");
        //                        Serial.print(stringValue.c_str());
        //                        Serial.println("]");

        //      if (((tt++) % 5) == 0)
        //      {
        //        LOG64_SET(F("BLE:REQUEST DISCARDED"));
        //        LOG64_NEW_LINE;
        //
        //        return;
        //      }

        strcpy(bleTransportRequestBuf, stringValue.c_str());
        // we do not fill the flag as we may be able to execute it on BLE thread
        // get oper
#define BLE_DELIMITER_END_OPER_COUNT 2
        uint16_t indexDelimiter = 0;
        uint16_t countDelimiters = 0;
        uint16_t prevDelimiterPos = 0;
        for (; indexDelimiter < BLE_MAX_BUF_SIZE_WITH_SEQ ; indexDelimiter++)
        {
          if ((bleTransportRequestBuf[indexDelimiter] == BLE_DELIMITER.c_str()[0]) || (bleTransportRequestBuf[indexDelimiter] == 0))
          {
            countDelimiters++;
            if (countDelimiters == BLE_DELIMITER_END_OPER_COUNT)
            {
              break;
            }
            prevDelimiterPos = indexDelimiter;
          }
        }
        if ((indexDelimiter >= BLE_MAX_BUF_SIZE_WITH_SEQ) || (BLE_DELIMITER_END_OPER_COUNT != countDelimiters) || (prevDelimiterPos == 0))
        {
          LOG64_SET(F("BLE: RECEIVED NO OPER IN REQUEST"));
          LOG64_NEW_LINE;
          return;
        }
        // temp set string end
        char tmpc = bleTransportRequestBuf[indexDelimiter];
        bleTransportRequestBuf[indexDelimiter] = 0;

        uint16_t oper = atoi(&bleTransportRequestBuf[prevDelimiterPos + 1]);
        // revert string end
        bleTransportRequestBuf[indexDelimiter] = tmpc;

        if (bleExecOnBLEThread(oper))
        {
          //          uint32_t ls = millis();
          //          Serial.println(ls - ss);
          char * bleTransportRequestTemp = bleTransportRequestBuf;
          const char * bleTransportResponseTemp = bleTransportResponseBuf;
          //          int8_t doLogResponse = bleProcessRequest(bleTransportRequestTemp, bleTransportResponseTemp);
          //          if (doLogResponse > == 0) { bleProcessResponse(doLogResponse, bleTransportResponseTemp); }
          if (bleProcessRequest(bleTransportRequestTemp, bleTransportResponseTemp) >= 0)
          {
            bleProcessResponse(0, bleTransportResponseTemp);
          }

          //          Serial.println(millis() - ls);
          //          ss = millis();


          return;
        }

        // fill the ready to process flag
        bleTransportRequest = bleTransportRequestBuf;
      }
      else
      {
        LOG64_SET(F("BLE: BUSY WITH PREV"));
        LOG64_NEW_LINE;
      }

      yield();
    }
};


inline void bleCleanup()
{
  //self cleanup if response it not sent in BLE_TRANSPORT_CLEANUP_TIMEOUT
  if (bleTransportResponse != NULL)
  {
    if (((uint32_t)(((uint32_t)millis()) - bleTransportResponseSet)) >= BLE_TRANSPORT_CLEANUP_TIMEOUT)
    {
      bleTransportResponse = NULL;
    }
  }
}

inline void bleProcessConnection()
{
  if (((uint32_t)(((uint32_t)millis()) - bleConnectionLast)) >= BLE_CONNECTION_TIMEOUT)
  {
    bleConnectionLast = millis();

    // disconnecting
    if (!bleConnected && bleOldConnected)
    {

      LOG64_SET(F("BLE: DISCONNECTED"));
      LOG64_NEW_LINE;

      bleOldConnected = bleConnected;
    }
    // connecting
    if (bleConnected && !bleOldConnected)
    {
      LOG64_SET(F("BLE: CONNECTED"));
      LOG64_NEW_LINE;
      bleOldConnected = bleConnected;
    }

    // reset connection if too long
    if (bleConnectedLast != 0)
    {
      if (((uint32_t)(((uint32_t)millis()) - bleConnectedLast)) >= BLE_DISCONNECT_TIMEOUT)
      {
        // start another monitor cycle - the clenup is in disconnect callback
        if ((bleConnectedLast = millis()) == 0 )
        {
          bleConnectedLast = 1;
        }
        LOG64_SET(F("BLE: FORCED DISCONNECT : TOO LONG WITH NO MESSAGES"));
        LOG64_NEW_LINE;
        bleServer->disconnect(bleServer->getConnId()); // force disconnect
      }
    }

    // readvertizing
    if (bleAdvertizingLast != 0)
    {
      if (((uint32_t)(((uint32_t)millis()) - bleAdvertizingLast)) >= BLE_READVERTIZING_TIMEOUT)
      {
        LOG64_SET(F("BLE: REFRESH ADVERTISING SKIPPED"));
        LOG64_NEW_LINE;
        //bleServer->startAdvertising(); // restart advertising
        if ((bleAdvertizingLast = millis()) == 0 )
        {
          bleAdvertizingLast = 1;
        }
      }
    }
  }
}

inline void run_ble()
{
  if (((uint32_t)(((uint32_t)millis()) - bleLastRun)) >= BLE_TIMEOUT_RUN)
  {
    bleLastRun = millis();

    bleProcessConnection();

    if (bleConnected)
    {
      int8_t doLogResponse = bleProcessRequest(bleTransportRequest, bleTransportResponse);
      if (doLogResponse >= 0)
      {
        bleProcessResponse(doLogResponse, bleTransportResponse);
      }
    }

    bleCleanup();

  }
}

void SetBLELog(const char * str)
{
  if (bleLogSeq >= BLE_MAX_SEQUNECE)
  {
    bleLogSeq = 0;
  }
  bool incSequence = false;
  bleLogSeq++;
  String seq = String(bleLogSeq);// + "." + String(((millis() / 1000) / 60) / 60); // add if required hours after last restart
  size_t seqSize = seq.length();
  size_t strSize = strlen (str);
  // check if cr/le
  if (strSize > 1)
  {
    if ((str[strSize - 2] == '\r') && (str[strSize - 1] == '\n'))
    {
      // line end we need sequence for the next
      incSequence = true;
    }
  }
  if ((bleLogSize + strSize + seqSize + 1 + 1 ) > (bleLogBufSize - 1))
  {
    // clean old log - no space
    bleLogSize = 0;
    // we need sequence for the next and fill with empty
    incSequence = true;
    str = "\r\n";
    strSize = 2;
  }

  if ((bleLogSize + strSize + seqSize + 1 + 1 ) > (bleLogBufSize - 1))
  {
    // too big skip
    // we need sequence for the next and fill with empty
    incSequence = true;
    str = "\r\n";
    strSize = 2;
  }

  memcpy(&bleLogBuf[bleLogSize], str, strSize);
  bleLogSize += strSize;
  if (incSequence)
  {
    memcpy(&bleLogBuf[bleLogSize], seq.c_str(), seqSize);
    bleLogSize += seqSize;
    bleLogBuf[bleLogSize++] = ':';
  }
  else
  {
    // revert sequnece
    bleLogSeq--;
  }
}

inline void init_ble()
{
  bleLastRun = millis();
  
  bleLogSeq = 0;
  bleLogSize = 0;
  bleConnectionLast = millis();
  bleConnectedLast = 0;
  bleAdvertizingLast = 0;

  bleServer = NULL;
  bleCharacteristicNonInteractive = NULL;
  bleCharacteristicInTransport = NULL;
  bleCharacteristicOutTransport = NULL;

  bleTransportResponse = NULL;
  bleTransportResponseSet = millis();

  bleTransportRequest = NULL;

  bleConnected = false;
  bleOldConnected = false;

  LOG64_SET(F("BLE: FIELDS COUNT : "));
  LOG64_SET((uint32_t)BLE_DATA_FIELDS_INDEX_COUNT);
  LOG64_NEW_LINE;

  // Create the BLE Device
  LOG64_SET(F("BLE: INIT : "));
  LOG64_SET(String(BLE_NAME_PREFIX + String(GetSetupDeviceName()) + ")"));
  LOG64_NEW_LINE;

  //    for (uint8_t i = 0; i < BLE_SETTING_DESC_COUNT; i++)
  //    {
  //      LOG64_SET("BLE:");
  //      LOG64_SET(strlen(BLE_SETTING_DESC[i]));
  //      LOG64_SET(BLE_SETTING_DESC[i]);
  //      LOG64_NEW_LINE;
  //    }


  // additional logs
  //    BLEDevice::setCustomGapHandler(my_gap_event_handler);
  //    BLEDevice::setCustomGattsHandler(my_gatts_event_handler);
  //    BLEDevice::setCustomGattcHandler(my_gattc_event_handler);

  BLEDevice::init(String(BLE_NAME_PREFIX + String(GetSetupDeviceName()) + ")").c_str());
  BLEDevice::setMTU(BLE_DEFAULT_MTU);
  // Create the BLE Server
  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new bleServiceCallbacks());

  // Create the BLE Service
  BLEService *pService = bleServer->createService(BLE_SERVICE_UUID);




  // Create input BLE Characteristic
  bleCharacteristicInTransport = pService->createCharacteristic(
                                   BLE_CHARACTERISTIC_TRANSPORT_IN_UUID,
                                   BLECharacteristic::PROPERTY_READ   |
                                   BLECharacteristic::PROPERTY_WRITE  |
                                   BLECharacteristic::PROPERTY_NOTIFY |
                                   BLECharacteristic::PROPERTY_INDICATE
                                 );

  bleCharacteristicInTransport->setCallbacks(new bleCharacteristicInTransportCallback());

  // Create a BLE Descriptor
  bleCharacteristicInTransport->addDescriptor(new BLE2902());


  // Create output BLE Characteristic
  bleCharacteristicOutTransport = pService->createCharacteristic(
                                    BLE_CHARACTERISTIC_TRANSPORT_OUT_UUID,
                                    BLECharacteristic::PROPERTY_READ   |
                                    BLECharacteristic::PROPERTY_WRITE  |
                                    BLECharacteristic::PROPERTY_NOTIFY |
                                    BLECharacteristic::PROPERTY_INDICATE
                                  );

  // Create a BLE Descriptor
  bleCharacteristicOutTransport->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  LOG64_SET(F("BLE: CONNECTED : START NOTIFY"));
  LOG64_NEW_LINE;
  // Start advertising


  bleServer->getAdvertising()->setScanResponse(true);
  bleServer->getAdvertising()->setMinPreferred(0x06);
  bleServer->getAdvertising()->setMaxPreferred(0x12);
  bleServer->getAdvertising()->start();

  if ((bleAdvertizingLast = millis()) == 0 )
  {
    bleAdvertizingLast = 1;
  }

}
