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


///////////////////////////////////////////////////////////////////////////////////
// Data store and read from EEPROM

#include <EEPROM.h>

///////////////////////////////////////////////////////////////////////////////////
// Defaults
#define SETUP_DEFAULT_SECRET 86
#define SETUP_DEFAULT_DEVICE_NAME "Gateway"
#define SETUP_DEFAULT_ID 5000
#define SETUP_DEFAULT_KEY "86868686868686868686868686868686"
#define SETUP_DEFAULT_FREQ 868.1f
#define SETUP_DEFAULT_SERVER "server.i4things.com"
#define SETUP_DEFAULT_PORT 5409
#define SETUP_DEFAULT_REPORT_TIMEOUT 60000

#define SETUP_DEFAULT_BAT_VOLTMETER_OFFSET 0.0f

#define SETUP_DEFAULT_GPRS_APN "iot.1nce.net"
#define SETUP_DEFAULT_GPRS_USER " "
#define SETUP_DEFAULT_GPRS_PASS " "

#define SETUP_DEFAULT_WIFI_SSID "N/A"
#define SETUP_DEFAULT_WIFI_PASS "N/A"

// GPRS
#define SETUP_DEFAULT_TRANSPORT 1

#define SETUP_DEFAULT_NODE_ID 1000
#define SETUP_DEFAULT_NODE_PRIV_KEY "06060606060606060606060606060606"

///////////////////////////////////////////////////////////////////////////////////
// Helper macro's for function definitions

#define SETUP_ORDINAL_VALUE(n, eepromN, t) \
  \
  t setup##n; \
  \
  inline void setupReadEeprom##n() \
  { \
    EEPROM.end(); \
    EEPROM.begin(EEPROM_SIZE); \
    \
    EEPROM.get(EEPROM_ADDRESS_##eepromN, setup##n); \
    \
    LOG64_SET(F("SETUP: READ FROM EEPROM " #eepromN "[")); \
    LOG64_SET(( t )setup##n); \
    LOG64_SET(F("]")); \
    LOG64_NEW_LINE; \
  } \
  \
  inline void setupWriteEeprom##n(t v) \
  { \
    EEPROM.end(); \
    EEPROM.begin(EEPROM_SIZE); \
    \
    EEPROM.put(EEPROM_ADDRESS_##eepromN, v); \
    \
    EEPROM.commit(); \
    EEPROM.end(); \
    EEPROM.begin(EEPROM_SIZE); \
    \
    setup##n = v; \
    \
    LOG64_SET(F("SETUP: WRITE TO EEPROM " #eepromN "[")); \
    LOG64_SET(setup##n); \
    LOG64_SET(F("]")); \
    LOG64_NEW_LINE; \
  } \
  \
  t GetSetup##n() \
  { \
    t ret; \
    ret = setup##n; \
    return ret; \
  } \
  \
  void SetSetup##n(t v) \
  { \
    setupWriteEeprom##n(v); \
  }


#define SETUP_LITERAL_VALUE(n, eepromN) \
  \
  char setup##n[eepromN##_MAX + 1]; \
  \
  inline void setupReadEeprom##n() \
  { \
    EEPROM.end(); \
    EEPROM.begin(EEPROM_SIZE); \
    \
    EEPROM.get(EEPROM_ADDRESS_##eepromN, setup##n); \
    \
    LOG64_SET(F("SETUP: READ FROM EEPROM " #eepromN "[")); \
    LOG64_SET(setup##n); \
    LOG64_SET(F("]")); \
    LOG64_NEW_LINE; \
  } \
  \
  inline void setupWriteEeprom##n(const char * v) \
  { \
    strcpy(setup##n, v);\
    \
    EEPROM.end(); \
    EEPROM.begin(EEPROM_SIZE); \
    \
    EEPROM.put(EEPROM_ADDRESS_##eepromN, setup##n); \
    \
    EEPROM.commit(); \
    EEPROM.end(); \
    EEPROM.begin(EEPROM_SIZE); \
    \
    LOG64_SET(F("SETUP: WRITE TO EEPROM " #eepromN "[")); \
    LOG64_SET(setup##n); \
    LOG64_SET(F("]")); \
    LOG64_NEW_LINE; \
  } \
  \
  const char * GetSetup##n() \
  { \
    return setup##n; \
  } \
  \
  void SetSetup##n(const char * v) \
  { \
    setupWriteEeprom##n(v); \
  }


///////////////////////////////////////////////////////////////////////////////////
// Define the following fuctions with XX = (PublisherTimeout, PosweOffVoltage, SiteId, DeviceId, Secret, WifiSsid, WifiPass, GprsApn, GprsUser, GprsPass )
//
//inline void setupReadEepromXX()
//inline void setupWriteEepromXX(T v)
//inline T GetSetupXX()
//inline void SetSetupXXX(T v)



SETUP_LITERAL_VALUE( DeviceName, DEVICE_NAME)
SETUP_ORDINAL_VALUE( Id, ID, uint32_t)
SETUP_LITERAL_VALUE( Key, KEY)
SETUP_ORDINAL_VALUE( Freq, FREQ, float)
SETUP_LITERAL_VALUE( Server, SERVER)
SETUP_ORDINAL_VALUE( Port, PORT, uint16_t)
SETUP_ORDINAL_VALUE( ReportTimeout, REPORT_TIMEOUT, uint32_t)

SETUP_ORDINAL_VALUE( BatVoltmeterOffset, BAT_VOLTMETER_OFFSET, float)

SETUP_LITERAL_VALUE( GprsApn, GPRS_APN)
SETUP_LITERAL_VALUE( GprsUser, GPRS_USER)
SETUP_LITERAL_VALUE( GprsPass, GPRS_PASS)

SETUP_LITERAL_VALUE( WifiSsid, WIFI_SSID)
SETUP_LITERAL_VALUE( WifiPass, WIFI_PASS)

SETUP_ORDINAL_VALUE( Transport, TRANSPORT, uint8_t)

SETUP_ORDINAL_VALUE( NodeId, NODE_ID, uint32_t)
SETUP_LITERAL_VALUE( NodePrivKey, NODE_PRIV_KEY)

SETUP_ORDINAL_VALUE( Secret, SECRET, uint8_t)

inline void init_setup()
{

  EEPROM.begin(EEPROM_SIZE);

  setupReadEepromDeviceName();
  setupReadEepromId();
  setupReadEepromKey();
  setupReadEepromFreq();
  setupReadEepromServer();
  setupReadEepromPort();
  setupReadEepromReportTimeout();

  setupReadEepromBatVoltmeterOffset();

  setupReadEepromGprsApn();
  setupReadEepromGprsUser();
  setupReadEepromGprsPass();

  setupReadEepromWifiSsid();
  setupReadEepromWifiPass();

  setupReadEepromTransport();

  setupReadEepromNodeId();
  setupReadEepromNodePrivKey();

  setupReadEepromSecret();

  // check if eeprom never has been intialized
  if (setupDeviceName[0] == 0xFF)
  {
    setupWriteEepromDeviceName(SETUP_DEFAULT_DEVICE_NAME);
  }
  if (setupId == 0xFFFFFFFF)
  {
    setupWriteEepromId(SETUP_DEFAULT_ID);
  }
  if (setupKey[0] == 0xFF)
  {
    setupWriteEepromKey(SETUP_DEFAULT_KEY);
  }
  if (isnan(setupFreq) || (setupFreq < 400.0f) || (setupFreq > (1000.0f)))
  {
    setupWriteEepromFreq(SETUP_DEFAULT_FREQ);
  }
  if (setupServer[0] == 0xFF)
  {
    setupWriteEepromServer(SETUP_DEFAULT_SERVER);
  }
  if (setupPort == 0xFFFF)
  {
    setupWriteEepromPort(SETUP_DEFAULT_PORT);
  }
  if (setupReportTimeout == 0xFFFFFFFF)
  {
    setupWriteEepromReportTimeout(SETUP_DEFAULT_REPORT_TIMEOUT);
  }
  if (isnan(setupBatVoltmeterOffset) || (setupBatVoltmeterOffset > 1.0f) || (setupBatVoltmeterOffset < (-1.0f)))
  {
    setupWriteEepromBatVoltmeterOffset(SETUP_DEFAULT_BAT_VOLTMETER_OFFSET);
  }
  if (setupGprsApn[0] == 0xFF)
  {
    setupWriteEepromGprsApn(SETUP_DEFAULT_GPRS_APN);
  }
  if (setupGprsUser[0] == 0xFF)
  {
    setupWriteEepromGprsUser(SETUP_DEFAULT_GPRS_USER);
  }
  if (setupGprsPass[0] == 0xFF)
  {
    setupWriteEepromGprsPass(SETUP_DEFAULT_GPRS_PASS);
  }
  if (setupWifiSsid[0] == 0xFF)
  {
    setupWriteEepromWifiSsid(SETUP_DEFAULT_WIFI_SSID);
  }
  if (setupWifiPass[0] == 0xFF)
  {
    setupWriteEepromWifiPass(SETUP_DEFAULT_WIFI_PASS);
  }
  if (setupTransport == 0xFF)
  {
    setupWriteEepromTransport(SETUP_DEFAULT_TRANSPORT);
  }
  if (setupNodeId == 0xFFFFFFFF)
  {
    setupWriteEepromNodeId(SETUP_DEFAULT_NODE_ID);
  }
  if (setupNodePrivKey[0] == 0xFF)
  {
    setupWriteEepromNodePrivKey(SETUP_DEFAULT_NODE_PRIV_KEY);
  }
  if (setupSecret == 0xFF)
  {
    setupWriteEepromSecret(SETUP_DEFAULT_SECRET);
  }
}
