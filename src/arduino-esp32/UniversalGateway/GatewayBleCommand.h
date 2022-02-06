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
// BLUETOOTH/BLE COMMANDS
//
// INPORTANT !!!
// INC REVISION WHEN CHANGING DEFINITIONS TO FORCE CLIENT TO UPDATE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HELPERS

#define BLE_OTA_BUF_SIZE 640
uint8_t otaBuf[BLE_OTA_BUF_SIZE];
uint32_t otaBufSize;

// decode from special chunk encoding (escaping and monimize 0's
inline void bleDecode(const char * src, uint8_t buf[], uint32_t & bufSize)
{
  //
  //  for (int j = 0; src[j] != 0; j++)
  //  {
  //    Serial.print((uint32_t)src[j]);
  //    Serial.print(" ");
  //
  //  }
  //
  //  Serial.println();

  bufSize = 0;
  uint32_t i = 0;
  for (; (src[i] != 0) && (bufSize <= BLE_OTA_BUF_SIZE) ;)
  {

    if (src[i] == 5)
    {
      i++;
      buf[bufSize++] = static_cast<uint8_t>(static_cast<uint8_t>(src[i]) - (86 + 1));
    }
    else
    {
      buf[bufSize++] = static_cast<uint8_t>(static_cast<uint8_t>(src[i]) - 86);
    }

    i++;
  }


}

inline bool bleCheckCharHex(char input)
{
  if (input >= '0' && input <= '9')
  {
    return true;
  }
  else if ((input >= 'A') && (input <= 'F'))
  {
    return true;
  }
  else if ((input >= 'a') && (input <= 'f'))
  {
    return true;
  }
  return false;
}

inline bool bleCheckStringKey(const char src[])
{
  int i = 0;
  for (; ; i++)
  {
    if (src[i] == 0)
    {
      break;
    }
    else if (!bleCheckCharHex(src[i]))
    {
      break;
    }
  }

  return (i == 32);
}

inline bool bleCheckCharDomain(char input)
{
  if (input >= '0' && input <= '9')
  {
    return true;
  }
  else if ((input >= 'A') && (input <= 'Z'))
  {
    return true;
  }
  else if ((input >= 'a') && (input <= 'z'))
  {
    return true;
  }
  else if (input >= '-')
  {
    return true;
  }
  else if (input >= '.')
  {
    return true;
  }

  return false;
}

inline bool bleCheckStringDomain(const char src[])
{
  for (int i = 0; ; i++)
  {
    if (src[i] == 0)
    {
      break;
    }
    else if (!bleCheckCharDomain(src[i]))
    {
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////////
// MSEC CALC
#define HOUR_MILLIS 3600000
#define MINUTE_MILLIS 60000
#define SECOND_MILLIS 1000

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BLE_DELIMITER   String('@')
#define BLE_ELEMENT_END String('#')

#define BLE_SETTING_DEVICE_NAME "Gateway Name"
#define BLE_SETTING_ID_NAME "Gateway Id"
#define BLE_SETTING_KEY_NAME "Gateway Key"
#define BLE_SETTING_FREQ_NAME "Gateway Frequency"
#define BLE_SETTING_SERVER_NAME "Server Address"
#define BLE_SETTING_PORT_NAME "Server Port"
#define BLE_SETTING_REPORT_NAME "Reporting Interval"

#define BLE_SETTING_BAT_VOLTMETER_OFFSET_NAME "Battery Voltmeter Calibration"

#define BLE_SETTING_APN_NAME "Apn"
#define BLE_SETTING_USER_NAME "User"
#define BLE_SETTING_PASSWORD_NAME "Password"

#define BLE_SETTING_WIFI_SSID_NAME "SSID"
#define BLE_SETTING_WIFI_KEY_NAME "Key"

#define BLE_SETTING_TRANSPORT_NAME "Transport"

#define BLE_SETTING_NODE_ID_NAME "Node Id"
#define BLE_SETTING_NODE_KEY_NAME "Node Key"

#define BLE_SETTING_SECRET_NAME "(0-255)"

#define BLE_REPORT_DEFAULT 3
#define BLE_REPORT_COUNT 9
float BLE_REPORT_FLOAT[BLE_REPORT_COUNT]
{
  30000.0f,
  60000.0f,
  90000.0f,
  120000.0f,
  150000.0f,
  180000.0f,
  1800000.0f,
  3600000.0f
};

const char * BLE_REPORT_STRING[BLE_REPORT_COUNT]
{
  "30s",
  "1m",
  "1m:30s",
  "2m",
  "2m:30s",
  "3m",
  "5m",
  "30m",
  "1h"
};

#define BLE_FREQ_DEFAULT 3
#define BLE_FREQ_COUNT 9
float BLE_FREQ_FLOAT[BLE_FREQ_COUNT]
{
  433.1f,
  433.3f,
  433.5f,
  868.1f,
  868.3f,
  868.5f,
  915.1f,
  915.3f,
  915.5f
};

const char * BLE_FREQ_STRING[BLE_FREQ_COUNT]
{
  "433.1 Mhz",
  "433.3 Mhz",
  "433.5 Mhz",
  "868.1 Mhz",
  "868.3 Mhz",
  "868.5 Mhz",
  "915.1 Mhz",
  "915.3 Mhz",
  "915.5 Mhz"
};

#define BLE_TRANSPORT_DEFAULT 0
#define BLE_TRANSPORT_COUNT 2
float BLE_TRANSPORT_FLOAT[BLE_TRANSPORT_COUNT]
{
  0.0f,
  1.0f
};

const char * BLE_TRANSPORT_STRING[BLE_TRANSPORT_COUNT]
{
  "WiFi",
  "Cellular"
};



#define BLE_VOLTMETER_OFFSET_DEFAULT 4
#define BLE_VOLTMETER_OFFSET_COUNT 9
float BLE_VOLTMETER_OFFSET_FLOAT[BLE_VOLTMETER_OFFSET_COUNT]
{
  -0.25f,
  -0.2f,
  -0.15f,
  -0.1f,
  0.0f,
  0.1f,
  0.15f,
  0.2f,
  0.25f,
};

const char * BLE_VOLTMETER_OFFSET_STRING[BLE_VOLTMETER_OFFSET_COUNT]
{
  "-0.25v",
  "-0.2v",
  "-0.15v",
  "-0.1v",
  "0.0v",
  "0.1v",
  "0.15v",
  "0.2v",
  "0.25v"
};


#define BLE_SETTING_COUNT 17
#define BLE_SETTING_DESC_COUNT 14
const char * BLE_SETTING_DESC[BLE_SETTING_DESC_COUNT] =
{
  "@" BLE_SETTING_DEVICE_NAME "@S16" , // Group: N/A Fields: Device Nameof type STRING - max 16 chars
  "@" BLE_SETTING_ID_NAME "@S10" , // Group: N/A Fields: Id type STRING - max 10 chars
  "@" BLE_SETTING_KEY_NAME "@S32" , // Group: N/A Fields: Key type STRING - max 32 chars
  "@" BLE_SETTING_FREQ_NAME  "@D(433.1 Mhz;433.3 Mhz;433.5 Mhz;868.1 Mhz;868.3 Mhz;868.5 Mhz;915.1 Mhz;915.3 Mhz;915.5 Mhz)" , // Group: N/A Fields: Frequency of type drop down
  "Server@" BLE_SETTING_SERVER_NAME "@S253@" BLE_SETTING_PORT_NAME "@S5", // Group: Server, Fields: Server type STRING - max 253 chars, Port type STRING - max 5 chars
  "@" BLE_SETTING_REPORT_NAME  "@D(30s;1m;1m:30s;2m;2m:30s;3m;5m;30m;1h)" , // Group: N/A Fields: Reporting Interval of type drop down - values in msec
  "@" BLE_SETTING_BAT_VOLTMETER_OFFSET_NAME "@D(-0.25v;-0.2v;-0.15v;-0.1v;0.0v;0.1v;0.15v;0.2v;0.25v)", // Group: N/A Fields: Voltmeter House offset of type drop down
  "Mobile@" BLE_SETTING_APN_NAME "@S63@" BLE_SETTING_USER_NAME "@S63@" BLE_SETTING_PASSWORD_NAME "@S63", // Group: Mobile , Fields:  Apn (max 253 chars), User(max 63 chars), Password(max 63 chars) of type STRING
  "WiFi@" BLE_SETTING_WIFI_SSID_NAME "@S32@" BLE_SETTING_WIFI_KEY_NAME "@S63", // Group: WiFi , Fileds: SSID(max 32 chars), Key(max 63 chars) of type STRING
  "@" BLE_SETTING_TRANSPORT_NAME "@D(WiFi;Cellular)" , // Group: N/A Fields: Trasport of type drop down
  "@" BLE_SETTING_NODE_ID_NAME "@S10" , // Group: N/A Fields: Node Id type STRING - max 10 chars
  "@" BLE_SETTING_NODE_KEY_NAME "@S32", // Group: N/A Fields: Node Key type STRING - max 32 chars
  "Secret@" BLE_SETTING_SECRET_NAME "@S3" // Group: Secret , Fileds: Secret in byte format(max 3 chars of type STRING
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DROP DOWN VALUES

float bleStringToFloat(const char * in,
                       uint8_t count, const char * stringData[], float floatData[], uint8_t defaultIndex)
{
  for (int i = 0; i < count; i++)
  {
    if (strcmp(in, stringData[i]) == 0)
    {
      return  floatData[i];
    }
  }
  return floatData[defaultIndex];
}

const char * bleFloatToString(float in,
                              uint8_t count, const char * stringData[], float floatData[], uint8_t defaultIndex)
{
  for (int i = 0; i < count; i++)
  {
    if (in == floatData[i])
    {
      return  stringData[i];
    }
  }
  return stringData[defaultIndex];
}


#define BLE_DATA_FIELDS_INDEX_COUNT 16

const char * BLE_DATA_FIELDS[BLE_DATA_FIELDS_INDEX_COUNT] =
{
  "Msg Recv", //0
  "Msg Sent", // 1
  "Reporting", // 2
  "Temp", //3
  "Humidity", //4
  "Lat", //5
  "Lon", // 6
  "Sat", // 7
  "WiFi", // 8
  "Radio", // 9
  "Cellular", // 10
  "Last", // 11
  "Next", // 12
  "Speed", //13
  "Transport", //14
  "Battery Voltmeter" // 15
};

#define BLE_OTA_OPERATION_COUNT 3

#define BLE_OTA_FIELDS_INDEX_COUNT 4

#define BLE_OTA_TYPE_WROOM32 "WROOM32"
#define BLE_OTA_TYPE_HELTEC32 "HELTEC32"

const char * BLE_OTA_FIELDS[BLE_OTA_FIELDS_INDEX_COUNT] =
{
  "OTAVersion", // 0 format "Ver.#3.20"
  "OTARevision", //1 format 1044
  "OTAType", //2  HELTEC, WROOM32
  "OTAUpdateSize" //3
};

#define BLE_NA "-"

#define BLE_SECRET_DATA_FIELD "Secret"

#define BLE_REVISION_DATA_FIELD "Revision"
#define BLE_REVISION "1"

#define BLE_DATA_FIELDS_BASE_GET_OPERATION 100
#define BLE_SETTING_GET_DESC_OPERATION 200
#define BLE_SETTING_GET_OPERATION 300
#define BLE_SETTING_SET_OPERATION 400
#define BLE_LOG_GET_OPERATION 500
#define BLE_REVISION_GET_OPERATION 600
#define BLE_OTA_GET_OPERATION 700
#define BLE_OTA_SET_OPERATION 800

#define BLE_FACTORY_OPERATION_COUNT 3
#define BLE_FACTORY_GET_OPERATION 2000
#define BLE_FACTORY_SET_OPERATION 3000

#define BLE_SECRET_GET_OPERATION 8606

inline bool bleExecOnBLEThread(uint16_t oper)
{
  if ((oper >= BLE_DATA_FIELDS_BASE_GET_OPERATION) && (oper < BLE_SETTING_GET_DESC_OPERATION))
  {
    return true;
  }
  if ((oper >= BLE_SETTING_GET_DESC_OPERATION) && (oper < BLE_SETTING_GET_OPERATION))
  {
    return true;
  }
  if ((oper >= BLE_SETTING_GET_OPERATION) && (oper < BLE_SETTING_SET_OPERATION))
  {
    return true;
  }
  if ((oper >= BLE_FACTORY_GET_OPERATION) && (oper < BLE_FACTORY_SET_OPERATION))
  {
    return true;
  }
  if (oper == BLE_REVISION_GET_OPERATION)
  {
    return true;
  }
  if (oper == BLE_SECRET_GET_OPERATION)
  {
    return true;
  }
  // all OTA
  if ((oper >= BLE_OTA_GET_OPERATION) && (oper < (BLE_OTA_SET_OPERATION + 100)))
  {
    return true;
  }

  return false;
}

inline bool bleGetFactory(uint16_t index, char buf[])
{
  String ble = String();

  switch (index)
  {
    case 0 :
      {
        ble += String(GetSetupId()) + BLE_ELEMENT_END;
        break;
      }
    case 1 :
      {
        ble += String(GetSetupNodeId()) + BLE_ELEMENT_END;
        break;
      }
    case 2 :
      {
        ble += String(GetSetupSecret()) + BLE_ELEMENT_END;
        break;
      }
  }

  if (ble.length() > 0)
  {
    strcpy(buf, ble.c_str());
    return true;
  }
  else
  {
    return false;
  }
}

inline void bleSetFactory(uint16_t index, char buf[])
{
  switch (index)
  {
    case 0 :
      {
        uint32_t id = atoi(buf);
        SetSetupId(id);
        break;
      }
    case 1 :
      {
        uint32_t node_id = atoi(buf);
        SetSetupNodeId(node_id);
        break;
      }
    case 2 :
      {
        uint8_t secret = static_cast<uint8_t>(atoi(buf));
        SetSetupSecret(secret);
        break;
      }
  }

}

inline bool bleGetOta(uint16_t index, char buf[])
{
  String ble = String();

  switch (index)
  {
    case 0 :
      {
        ble +=  String(F( Version_Major_Minor )) + BLE_ELEMENT_END;
        break;
      }
    case 1 :
      {
        ble += String(F( Version_Revision )) + BLE_ELEMENT_END;
        break;
      }
    case 2 :
      {
#if defined(WROOM32)
        ble +=  String(F( BLE_OTA_TYPE_WROOM32 )) + BLE_ELEMENT_END;
#else
        ble +=  String(F( BLE_OTA_TYPE_HELTEC32)) + BLE_ELEMENT_END;
#endif
        break;
      }

    case 3 :
      {
        ble +=  String(GetOTAUpdateSize()) + BLE_ELEMENT_END;
        break;
      }
  }

  if (ble.length() > 0)
  {
    strcpy(buf, ble.c_str());
    return true;
  }
  else
  {
    return false;
  }
}

inline void bleSetOta(uint16_t index, char buf[], char ret[])
{
  switch (index)
  {
    case 0 :
      {
        OTABegin(buf);
        break;
      }
    case 1 :
      {
        // we use binary format to optimize for speed
        //size_t len = strlen(buf);
        //if ((len > 0) && ((len % 2) == 0))
        //{
        //bleHex2Bin(buf, otaBuf, otaBufSize);
        bleDecode(buf, otaBuf, otaBufSize); // we use binary format now
        //
        uint16_t size = (static_cast<uint16_t>(otaBuf[0]) << 8) | static_cast<uint16_t>(otaBuf[1]);

        //          LOG64_SET(F("BLE: TRANSPORT SIZE ["));
        //          LOG64_SET(size);
        //          LOG64_SET((otaBufSize - 2));
        //          LOG64_SET(F("]"));
        //          LOG64_NEW_LINE;

        if (size != (otaBufSize - 2))
        {
          LOG64_SET(F("BLE: TRANSPORT WRONG SIZE ["));
          LOG64_SET(size);
          LOG64_SET((otaBufSize - 2));
          LOG64_SET(F("]"));
          LOG64_NEW_LINE;
        }
        else
        {
          uint32_t pos = (static_cast<uint32_t>(otaBuf[2]) << 24) + (static_cast<uint32_t>(otaBuf[3]) << 16) + (static_cast<uint32_t>(otaBuf[4]) << 8) + static_cast<uint32_t>(otaBuf[5]);

          //            LOG64_SET(F("BLE: TRANSPORT POS ["));
          //            LOG64_SET(pos);
          //            LOG64_SET(F("]"));
          //            LOG64_NEW_LINE;

          OTAWrite(&otaBuf[6], otaBufSize - 6, pos);
          //}
        }
        break;
      }
    case 2 :
      {
        OTAEnd();
        break;
      }
  }

  itoa (GetOTAUpdateSize(), ret, 10);
}



inline void bleSetSetting(uint16_t index, char buf[], char ret[])
{
  String ble = String();

  switch (index)
  {
    case 0 :
      {
        SetSetupDeviceName(buf);
        esp_ble_gap_set_device_name(String(BLE_NAME_PREFIX + String(GetSetupDeviceName()) + ")").c_str());
        break;
      }
    case 1 :
      {

        int id = atoi(buf);
        if (id < 1)
        {
          ble += BLE_ELEMENT_END + String("Please enter value in range [1 - 2147483647]");
        }
        else
        {
          SetSetupId(static_cast<uint32_t>(id));
          if (gateway != NULL)
          {
            gateway->set_id(static_cast<uint32_t>(id));
          }
        }
        break;
      }
    case 2 :
      {
        if (!bleCheckStringKey(buf))
        {
          ble += BLE_ELEMENT_END + String("Please enter 32 chars HEX");
        }
        else
        {
          SetSetupKey(buf);
          if (gateway != NULL)
          {
            uint8_t gateway_key[16];
            hex_to_key(buf, gateway_key);
            gateway->set_key(gateway_key);
          }
        }
        break;
      }
    case 3 :
      {
        float v = bleStringToFloat(buf, BLE_FREQ_COUNT, BLE_FREQ_STRING, BLE_FREQ_FLOAT, BLE_FREQ_DEFAULT);
        SetSetupFreq(v);
        if (gateway != NULL)
        {
          gateway->set_freq(v);
        }
        break;
      }
    case 4 :
      {
        if (!bleCheckStringDomain(buf))
        {
          ble += BLE_ELEMENT_END + String("Please enter domain [0-9,a-z,A-Z,-,.]");
        }
        else
        {
          SetSetupServer(buf);
          // server prt will be set in group e.g. port last
        }
        break;
      }
    case 5 :
      {

        int port = atoi(buf);
        if ((port < 1) || (port > 65535))
        {
          ble += BLE_ELEMENT_END + String("Please enter value in range [1 - 65535]");
        }
        else
        {
          SetSetupPort((uint16_t)port);
          if (server_wifi != NULL)
          {
            server_wifi->set_server_port(GetSetupServer(), GetSetupPort());
          }
        }
        break;
      }
    case 6 :
      {
        float v = bleStringToFloat(buf, BLE_REPORT_COUNT, BLE_REPORT_STRING, BLE_REPORT_FLOAT, BLE_REPORT_DEFAULT);
        SetSetupReportTimeout(v);
        break;
      }
    case 7 :
      {
        float f = bleStringToFloat(buf, BLE_VOLTMETER_OFFSET_COUNT, BLE_VOLTMETER_OFFSET_STRING, BLE_VOLTMETER_OFFSET_FLOAT, BLE_VOLTMETER_OFFSET_DEFAULT);
        SetSetupBatVoltmeterOffset(f);
        break;
      }
    case 8 :
      {
        if (!bleCheckStringDomain(buf))
        {
          ble += BLE_ELEMENT_END + String("Please enter APN [0-9,a-z,A-Z,-,.]");
        }
        else
        {
          SetSetupGprsApn(buf);
          //set as group
        }
        break;
      }
    case 9 :
      {
        SetSetupGprsUser(buf);
        // set as group
        break;
      }
    case 10 :
      {
        SetSetupGprsPass(buf);
        if (server_gprs != NULL)
        {
          server_gprs->set_apn_user_pass(GetSetupGprsApn(), GetSetupGprsUser(), GetSetupGprsPass());
        }
        break;
      }
    case 11 :
      {
        SetSetupWifiSsid(buf);
        // ssi and pass ae set in group  e.g. pass is last
        break;
      }
    case 12 :
      {
        SetSetupWifiPass(buf);
        if (server_wifi != NULL)
        {
          server_wifi->set_ssid_pass(GetSetupWifiSsid(), GetSetupWifiPass());
        }
        break;
      }
    case 13 :
      {
        float f = bleStringToFloat(buf, BLE_TRANSPORT_COUNT, BLE_TRANSPORT_STRING, BLE_TRANSPORT_FLOAT, BLE_TRANSPORT_DEFAULT);
        if (static_cast<uint8_t>(f) != GetSetupTransport())
        {
          SetSetupTransport(static_cast<uint8_t>(f));
          if (GetSetupTransport() == 0)
          {
            // wifi
            change_to_server_wifi();
          }
          else
          {
            // gprs
            change_to_server_gprs();
          }
        }
        break;
      }
    case 14 :
      {
        int id = atoi(buf);
        if (id < 1)
        {
          ble += BLE_ELEMENT_END + String("Please enter value in range [1 - 2147483647]");
        }
        else
        {
          SetSetupNodeId(static_cast<uint32_t>(id));
          if (thing_embedded != NULL)
          {
            thing_embedded->set_id(static_cast<uint32_t>(id));
          }
        }
        break;
      }
    case 15 :
      {
        if (!bleCheckStringKey(buf))
        {
          ble += BLE_ELEMENT_END + String("Please enter 32 chars HEX");
        }
        else
        {
          SetSetupNodePrivKey(buf);
          if (thing_embedded != NULL)
          {
            uint8_t node_key[16];
            hex_to_key(buf, node_key);
            thing_embedded->set_key(node_key);
          }
        }
        break;
      }
    case 16 :
      {
        int secret = atoi(buf);
        if ((secret < 1) || (secret > 255))
        {
          ble += BLE_ELEMENT_END + String("Please enter value in range [1 - 255]");
        }
        else
        {
          SetSetupSecret(static_cast<uint8_t>(secret));
        }
        break;
      }
  }

  strcpy(ret, ble.c_str());
}

inline bool bleGetSetting(uint16_t index, char buf[])
{
  String ble = String();

  switch (index)
  {
    case 0 :
      {
        ble += GetSetupDeviceName() + BLE_ELEMENT_END;
        break;
      }
    case 1 :
      {
        ble += String(GetSetupId()) + BLE_ELEMENT_END;
        break;
      }
    case 2 :
      {
        ble += String(GetSetupKey()) + BLE_ELEMENT_END;
        break;
      }
    case 3 :
      {
        const char * v = bleFloatToString(GetSetupFreq(), BLE_FREQ_COUNT, BLE_FREQ_STRING, BLE_FREQ_FLOAT, BLE_FREQ_DEFAULT);
        ble += String(v) + BLE_ELEMENT_END;
        break;
      }
    case 4 :
      {
        ble += String(GetSetupServer()) + BLE_ELEMENT_END;
        break;
      }
    case 5 :
      {
        ble += String(GetSetupPort()) + BLE_ELEMENT_END;
        break;
      }
    case 6 :
      {
        const char * v = bleFloatToString(GetSetupFreq(), BLE_REPORT_COUNT, BLE_REPORT_STRING, BLE_REPORT_FLOAT, BLE_REPORT_DEFAULT);
        ble += String(v) + BLE_ELEMENT_END;
        break;
      }
    case 7 :
      {
        const char * v = bleFloatToString(GetSetupBatVoltmeterOffset(), BLE_VOLTMETER_OFFSET_COUNT, BLE_VOLTMETER_OFFSET_STRING, BLE_VOLTMETER_OFFSET_FLOAT, BLE_VOLTMETER_OFFSET_DEFAULT);
        ble += String(v) + BLE_ELEMENT_END;
        break;
      }
    case 8 :
      {
        ble += GetSetupGprsApn() + BLE_ELEMENT_END;
        break;
      }
    case 9 :
      {
        ble += GetSetupGprsUser() + BLE_ELEMENT_END;
        break;
      }
    case 10 :
      {
        ble += GetSetupGprsPass() + BLE_ELEMENT_END;
        break;
      }
    case 11 :
      {
        ble += GetSetupWifiSsid() + BLE_ELEMENT_END;
        break;
      }
    case 12 :
      {
        ble += GetSetupWifiPass() + BLE_ELEMENT_END;
        break;
      }
    case 13 :
      {
        const char * v = bleFloatToString(GetSetupTransport(), BLE_TRANSPORT_COUNT, BLE_TRANSPORT_STRING, BLE_TRANSPORT_FLOAT, BLE_TRANSPORT_DEFAULT);
        ble += String(v) + BLE_ELEMENT_END;
        break;
      }
    case 14 :
      {
        ble += String(GetSetupNodeId()) + BLE_ELEMENT_END;
        break;
      }
    case 15 :
      {
        ble += String(GetSetupNodePrivKey()) + BLE_ELEMENT_END;
        break;
      }
    case 16 :
      {
        ble += String(GetSetupSecret()) + BLE_ELEMENT_END;
        break;
      }
  }

  if (ble.length() > 0)
  {
    strcpy(buf, ble.c_str());
    return true;
  }
  else
  {
    return false;
  }
}


inline bool bleGetCommand(uint16_t index, char buf[])
{

  String ble = String();

  switch (index)
  {
    case 0 :
      {
        ble += String(total_received) + BLE_ELEMENT_END;
        break;
      }
    case 1 :
      {
        if (GetSetupNodeId() == 0)
        {
          ble += "WiFi : " + String(total_received) + BLE_ELEMENT_END;
        }
        else
        {
          ble += "Cellular : " + String(total_received) + BLE_ELEMENT_END;
        }
        break;
      }
    case 2 :
      {
        uint32_t r = GetSetupReportTimeout();

        uint32_t r_h = (r / HOUR_MILLIS);
        uint32_t r_m = (r % HOUR_MILLIS) / MINUTE_MILLIS;
        uint32_t r_s = ((r % HOUR_MILLIS) % MINUTE_MILLIS) / SECOND_MILLIS;

        ble += ((r_h < 10) ? "0" + String(r_h) : String(r_h)) + ":" + ((r_m < 10) ? "0" + String(r_m) : String(r_m)) + ":" + ((r_s < 10) ? "0" + String(r_s) : String(r_s)) + BLE_ELEMENT_END;
        break;
      }
    case 3 :
      {
        float temperature = get_temperature();
        ble += ((isnan(temperature)) ? BLE_NA : String(temperature, 1) + " C") + BLE_ELEMENT_END;
        break;
      }
    case 4 :
      {
        uint8_t humidity = get_humidity();
        ble += ((isnan(humidity)) ? BLE_NA : String(humidity) + "%") + BLE_ELEMENT_END;
        break;
      }
    case 5 :
      {
        ble += String(get_gps_lat(), 6) + BLE_ELEMENT_END;
        break;
      }
    case 6:
      {
        ble += String(get_gps_lng(), 6) + BLE_ELEMENT_END;
        break;
      }
    case 7 :
      {
        ble += String(get_gps_satellite_count()) + BLE_ELEMENT_END;
        break;
      }
    case 8 :
      {
        ble += String((server_wifi == NULL) ? BLE_NA :  String(server_wifi->signal_strength()).c_str()) + "%" +  BLE_ELEMENT_END;
        break;
      }
    case 9 :
      {
        ble += String((gateway == NULL) ? BLE_NA : String(gateway->signal_strength()).c_str()) + "%" +  BLE_ELEMENT_END;
        break;
      }

    case 10 :
      {
        String((server_gprs == NULL) ? BLE_NA : String(server_gprs->signal_strength()).c_str()) + "%" +  BLE_ELEMENT_END;
        break;
      }
    case 11 :
      {
        uint32_t last = last_send();
        uint32_t l_m = (last % HOUR_MILLIS) / MINUTE_MILLIS;
        uint32_t l_s = ((last % HOUR_MILLIS) % MINUTE_MILLIS) / SECOND_MILLIS;

        ble += (((l_m < 10) ? "0" + String(l_m) : String(l_m)) + ":" + ((l_s < 10) ? "0" + String(l_s) : String(l_s))) + BLE_ELEMENT_END;
        break;
      }
    case 12 :
      {
        uint32_t next =  next_send();
        uint32_t n_m = (next % HOUR_MILLIS) / MINUTE_MILLIS;
        uint32_t n_s = ((next % HOUR_MILLIS) % MINUTE_MILLIS) / SECOND_MILLIS;

        ble += (((n_m < 10) ? "0" + String(n_m) : String(n_m)) + ":" + ((n_s < 10) ? "0" + String(n_s) : String(n_s))) +  BLE_ELEMENT_END;
        break;
      }
    case 13 :
      {
        ble += String(get_gps_speed()) + " knt" + BLE_ELEMENT_END;
        break;
      }
    case 14 :
      {
        const char * v = bleFloatToString(GetSetupTransport(), BLE_TRANSPORT_COUNT, BLE_TRANSPORT_STRING, BLE_TRANSPORT_FLOAT, BLE_TRANSPORT_DEFAULT);
        ble += String(v) + BLE_ELEMENT_END;
        break;
      }
    case 15 :
      {
        ble += ((isnan(get_bat()) ? BLE_NA : String(get_bat(), 2)) + "v") + BLE_ELEMENT_END;
        break;
      }
  }

  if (ble.length() > 0)
  {
    strcpy(buf, ble.c_str());
    return true;
  }
  else
  {
    return false;
  }
}
