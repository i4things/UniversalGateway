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

//To compile/upload to Heltec_Wifi_LoRa_32 or ESP32 Dev Module (make sure you have the proper define in "module_config":
//
//1: use the default.csv from the source code directory and place it is : C:\Users\admin\Documents\Arduino\hardware\espressif\esp32\tools\partitions  or (C:\Users\admin\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.0\tools\partitions)
//2: edit the following section in C:\Users\admin\Documents\Arduino\hardware\espressif\esp32\boards.txt :
//   to contain :
//heltec_wifi_lora_32.upload.maximum_size=2031616
// and
//m5stack-core-esp32.upload.maximum_size=2031616
// and
//esp32.name=ESP32 Dev Module
//
//esp32.upload.tool=esptool_py
//esp32.upload.maximum_size=2031616
//
// 3: Please update the following in (inc BT stack size): C:\Users\admin\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\tools\sdk\include\esp32\esp_task.h
//
///* Bt contoller Task */
///* controller */
//#define ESP_TASK_BT_CONTROLLER_PRIO   (ESP_TASK_PRIO_MAX - 2)
//#ifdef CONFIG_NEWLIB_NANO_FORMAT
//#define TASK_EXTRA_STACK_SIZE      (0) <<<< !!!
//#else
//#define TASK_EXTRA_STACK_SIZE      (512) <<<< !!!
//#endif
//
//#define BT_TASK_EXTRA_STACK_SIZE      TASK_EXTRA_STACK_SIZE
//#define ESP_TASK_BT_CONTROLLER_STACK  (3584 + TASK_EXTRA_STACK_SIZE)
//
//  with :
//
//  /* Bt contoller Task */
///* controller */
//#define ESP_TASK_BT_CONTROLLER_PRIO   (ESP_TASK_PRIO_MAX - 2)
//#ifdef CONFIG_NEWLIB_NANO_FORMAT
//#define TASK_EXTRA_STACK_SIZE      (4096)
//#else
//#define TASK_EXTRA_STACK_SIZE      (4096)
//#endif
//
//#define BT_TASK_EXTRA_STACK_SIZE      TASK_EXTRA_STACK_SIZE
//#define ESP_TASK_BT_CONTROLLER_STACK  (3584 + TASK_EXTRA_STACK_SIZE)
//
// 4: MAKE SURE you re using the RadioHead from svn - libraries and not the one from the official provider
// 5: restart arduino UI

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Optimize for size
// #pragma GCC optimize ("Os")

//#define WROOM32
///////////////////////////////////////////////////////////////////////////////////
// LOG constants

void SetBLELog(const char * l);
#define LOG64_ENABLED
#define LOG64_LOGX SetBLELog
#include <Log64.h>


#define Version_Major_Minor "Ver.#3.20"
// 20000 + svn revision
#define Version_Revision "21392"

///////////////////////////////////////////////////////////////////////////////////
// HELPERS
inline uint8_t chat_int(char input)
{
  if (input >= '0' && input <= '9')
  {
    return input - '0';
  }
  else if ((input >= 'A') && (input <= 'F'))
  {
    return input - 'A' + 10;
  }
  else if ((input >= 'a') && (input <= 'f'))
  {
    return input - 'a' + 10;
  }
  return 0;
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-fA-F] characters, and target to be sufficiently large
inline void hex_to_key(const char src[], uint8_t target[16])
{
  for (int i = 0; i < 16; i++)
  {
    target[i] = (chat_int(src[i << 1]) << 4) | chat_int(src[(i << 1) + 1]);
  }
}



//////////////////////////////////////////////////////////////////////////////////
//OTA constats

#define OTA_ERASE_SIZE (512 * 1024)

///////////////////////////////////////////////////////////////////////////////////
// GLOBALS
#include "IoTGateway.h"
IoTGateway *gateway;

#include "IoTServerWifi.h"
IoTServerWifi *server_wifi = NULL;

#include "IoTServerGprs.h"
IoTServerGprs *server_gprs = NULL;

#include "IoTThingEmbedded.h"
IoTThingEmbedded *thing_embedded = NULL;

///////////////////////////////////////////////////////////////////////////////////
// CALLBACKS

uint32_t total_received;
uint32_t total_send;

// called when wifi/gprs deserialzie a packet
void dispatch_data(uint8_t buf_[], uint8_t start_index_, uint8_t size_)
{
  gateway->add_dispatch_buffer(buf_, start_index_, size_);
}

// called when packet received from node
void received_data(uint8_t buf[], uint8_t size, int16_t rssi)
{
  
  total_received++;

  uint8_t buf_transport[128];
  uint8_t size_transport;

  gateway->gen_internet_message(buf_transport, size_transport, buf, size, rssi);

  total_send++;

  if (server_wifi != NULL)
  {
    server_wifi->send(buf_transport, size_transport);
  }

  if (server_gprs != NULL)
  {
    server_gprs->send(buf_transport, size_transport);
  }

};

// called when packet received from embedded node
void received_data_embedded(uint8_t buf[], uint8_t size, int16_t rssi)
{
  
  // treat it the same as non embeded
  received_data(buf, size, rssi);

  // notify that data has been scheduled for the server
  thing_embedded->data_sent_successfully();
};

///////////////////////////////////////////////////////////////////////////////////
// FORWARD INCLUDE
#include "GatewayPins.h"
#include "GatewayEEPROM.h"
#include "GatewaySetup.h"
///////////////////////////////////////////////////////////////////////////////////
// TRANSPORT HELPERS

inline void init_server_gprs()
{
  server_gprs = new IoTServerGprs(GetSetupGprsApn(), GetSetupGprsUser(), GetSetupGprsPass(), 1, GPRS_RX_PIN, GPRS_TX_PIN, GPRS_POWER_CONTROL_PIN,
#if defined(WROOM32)
                                  true,
#else
                                  false,
#endif

                                  GetSetupServer(), GetSetupPort(), &dispatch_data);
  server_gprs->init();
}

inline void init_server_wifi()
{
  server_wifi = new IoTServerWifi(GetSetupWifiSsid(), GetSetupWifiPass(), GetSetupServer(), GetSetupPort(), &dispatch_data);
  server_wifi->init();
}

inline void change_to_server_wifi()
{
  server_gprs->discard();
  delete server_gprs;
  server_gprs = NULL;

  // clear statistic
  total_received = 0;
  total_send = 0;

  init_server_wifi();
}

inline void change_to_server_gprs()
{
  server_wifi->discard();
  delete server_wifi;
  server_wifi = NULL;

  // clear statistic
  total_received = 0;
  total_send = 0;

  init_server_gprs();
}

///////////////////////////////////////////////////////////////////////////////////
// INCLUDE
#include "GatewaySHA1.h"
#include "GatewayMathSafe.h"
#include "GatewayOTA.h"
#include "GatewayMonitor.h"
#include "GatewayBat.h"
#include "GetewayDHT.h"
#include "GatewayGPS.h"
#include "GatewayNode.h"
#include "GatewayHeartbeat.h"
#include "GatewayBle.h"



void setup()
{

  // statistic
  total_received = 0;
  total_send = 0;

  // TIME ZONE TO GMT
  setenv("TZ", "GMT0", 1);
  tzset();

  // Init random generator
  randomSeed(analogRead(0));

  // Serial Log
  LOG64_INIT();
  //give change to usb serial to connect
#if defined(LOG64_ENABLED)
#define SERIAL_MAX_START_TIMEOUT 5000
  for (uint32_t start = millis(); (!Serial);)
  {
    yield();

    if (((uint32_t)(((uint32_t)millis()) - start)) >= SERIAL_MAX_START_TIMEOUT)
    {
      break;
    }
  }
#else
  delay(2000);
#endif

  // Print version
  LOG64_SET(F( Version_Major_Minor "." Version_Revision ));
  LOG64_NEW_LINE;

  // EEPROM
  init_eeprom();

  // Init & Erase OTA update partion
  //OTADumpPartitions();
  OTAInit();
  // Optional erase the future update partition
  //OTAEraseFull();

  //Monitor
  init_monitor();

  // Setup
  init_setup();

  // Batttery
  init_bat();

  // DHT
  init_dht();

  // GPS
  init_gps();


  // Embedded Node
  uint8_t thing_key[16];
  hex_to_key(GetSetupNodePrivKey(), thing_key);
  // dump node priv key
  LOG64_SET(F("NODE: KEY["));
  for (uint8_t i = 0; i < 16; i++)
  {
    LOG64_SET((uint32_t)thing_key[i]);
  }
  LOG64_SET(F("]"));
  LOG64_NEW_LINE;
  // end dump
  thing_embedded = NULL;
  thing_embedded = new IoTThingEmbedded(static_cast<uint64_t>( GetSetupNodeId()), thing_key, static_cast<uint64_t>( GetSetupId()), &received_data_embedded);
  thing_embedded->init();

  // Radio Gateway
  uint8_t gateway_key[16];
  hex_to_key(GetSetupKey(), gateway_key);
  gateway = NULL;
  gateway = new IoTGateway(CS_PIN, INT_PIN, RST_PIN, &received_data,  gateway_key, static_cast<uint64_t>( GetSetupId()));
  gateway->init(GetSetupFreq());

  //Transport ( on startup)
  if (GetSetupTransport() == 0)
  {
    // wifi
    server_gprs = NULL;
    init_server_wifi();
  }
  else
  {
    // gprs
    server_wifi = NULL;
    init_server_gprs();
  }

  //  // Node
  init_node();

  // BLE
  init_ble();

  //  // Heartbeat
  init_heartbeat();

}

#define EXECUTE_WITH_MONITOR( o_func, o_name ) \
  { \
    uint32_t t = millis(); \
    \
    o_func (); \
    \
    uint32_t delta = ((uint32_t)millis()) - t; \
    if (delta > 1) \
    { \
      LOG64_SET(#o_name " TOOK MSEC: ");\
      LOG64_SET(delta); \
      LOG64_NEW_LINE; \
    } \
  }

///////////////////////////////////////////////////////////////////////////////////
// MAIN LOOP
void loop()
{

  EXECUTE_WITH_MONITOR( run_monitor , MONITOR )

  EXECUTE_WITH_MONITOR( run_bat , BAT )

  EXECUTE_WITH_MONITOR( run_dht , DHT )

  EXECUTE_WITH_MONITOR( run_gps , GPS )

  EXECUTE_WITH_MONITOR( run_heartbeat , HEARTBEAT )

  EXECUTE_WITH_MONITOR( run_node , NODE )

  EXECUTE_WITH_MONITOR( run_ble , BLE )

  EXECUTE_WITH_MONITOR( thing_embedded->work , THING )

  EXECUTE_WITH_MONITOR( gateway->work , GATEWAY )

  if (server_wifi != NULL)
  {
    EXECUTE_WITH_MONITOR( server_wifi->work , SERVER_WIFI )
  }

  if (server_gprs != NULL)
  {
    EXECUTE_WITH_MONITOR( server_gprs->work , SERVER_GPRS )
  }




  yield();
}
