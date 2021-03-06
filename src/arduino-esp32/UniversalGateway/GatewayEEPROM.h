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
// EPROM ADDRESS DEFINITIONS

#define KEY_MAX 32
#define SERVER_MAX 253

#define NODE_PRIV_KEY_MAX 32

#define WIFI_SSID_MAX 32
#define WIFI_PASS_MAX 63

#define GPRS_APN_MAX 253
#define GPRS_USER_MAX 63
#define GPRS_PASS_MAX 63

#define DEVICE_NAME_MAX 63

#define EEPROM_ADDRESS_SECRET 0
#define EEPROM_ADDRESS_DEVICE_NAME (EEPROM_ADDRESS_SECRET + 1)
#define EEPROM_ADDRESS_ID (EEPROM_ADDRESS_DEVICE_NAME + DEVICE_NAME_MAX + 1)
#define EEPROM_ADDRESS_KEY (EEPROM_ADDRESS_ID + 8)
#define EEPROM_ADDRESS_FREQ (EEPROM_ADDRESS_KEY + KEY_MAX + 1)
#define EEPROM_ADDRESS_SERVER (EEPROM_ADDRESS_FREQ + SERVER_MAX + 1)
#define EEPROM_ADDRESS_PORT (EEPROM_ADDRESS_SERVER + SERVER_MAX + 1)
#define EEPROM_ADDRESS_REPORT_TIMEOUT (EEPROM_ADDRESS_PORT + 2)

#define EEPROM_ADDRESS_BAT_VOLTMETER_OFFSET (EEPROM_ADDRESS_REPORT_TIMEOUT + 4)

#define EEPROM_ADDRESS_GPRS_APN (EEPROM_ADDRESS_BAT_VOLTMETER_OFFSET + 4)
#define EEPROM_ADDRESS_GPRS_USER (EEPROM_ADDRESS_GPRS_APN + GPRS_APN_MAX + 1)
#define EEPROM_ADDRESS_GPRS_PASS (EEPROM_ADDRESS_GPRS_USER + GPRS_USER_MAX + 1)

#define EEPROM_ADDRESS_WIFI_SSID (EEPROM_ADDRESS_GPRS_PASS + GPRS_PASS_MAX + 1 )
#define EEPROM_ADDRESS_WIFI_PASS (EEPROM_ADDRESS_WIFI_SSID + WIFI_SSID_MAX + 1)

#define EEPROM_ADDRESS_TRANSPORT (EEPROM_ADDRESS_WIFI_PASS + WIFI_PASS_MAX + 1)

// embeded node
#define EEPROM_ADDRESS_NODE_ID (EEPROM_ADDRESS_TRANSPORT + 1)
#define EEPROM_ADDRESS_NODE_PRIV_KEY (EEPROM_ADDRESS_NODE_ID + 8)

// calc                                                                                                + 1)
#define EEPROM_END (EEPROM_ADDRESS_NODE_PRIV_KEY + NODE_PRIV_KEY_MAX + 1)
#define EEPROM_SIZE 2048

inline void init_eeprom()
{
  uint32_t eeprom_used = EEPROM_END;
  LOG64_SET(F("EEPROM: INIT ["));
  LOG64_SET(eeprom_used );
  LOG64_SET(F("] USED FROM ["));
  LOG64_SET((uint32_t)EEPROM_SIZE);
  LOG64_SET(F("]"));
  LOG64_NEW_LINE;
}
