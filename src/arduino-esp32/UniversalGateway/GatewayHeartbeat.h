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
// Hearthbeat / force downstream data get

// 5 min
#define HEARTBEAT_TIMEOUT  180000
uint32_t heartbeat_last_execute;

#define HEARTBEAT_MAX_MESSAGE_LEN 16
#define HEARTBEAT_OPER 127
#define HEARTBEAT_MAGIC 6

uint8_t heartbeat_magic_counter;

inline void init_heartbeat()
{
  heartbeat_last_execute = millis();
  heartbeat_magic_counter = 0;

  LOG64_SET(F("HEARTBEAT: INIT"));
  LOG64_NEW_LINE;
}


inline void send_message_heartbeat()
{
  if (gateway != NULL)
  {

    uint8_t buf[HEARTBEAT_MAX_MESSAGE_LEN];
    uint8_t size = 0;


    // gen packet
    size = 0;

    uint16_t magic = HEARTBEAT_MAGIC;
    magic = magic << 8;
    magic += (heartbeat_magic_counter++);

    buf[size++] = 11;

    buf[size++] = HEARTBEAT_OPER;

    buf[size++] = ((uint8_t*)&magic)[0];
    buf[size++] = ((uint8_t*)&magic)[1];

    uint32_t gateway_id = GetSetupId();
    buf[size++] = ((uint8_t*)&gateway_id)[0];
    buf[size++] = ((uint8_t*)&gateway_id)[1];
    buf[size++] = ((uint8_t*)&gateway_id)[2];
    buf[size++] = ((uint8_t*)&gateway_id)[3];

    uint16_t last_packet_took_to_sent = 0;
    buf[size++] = ((uint8_t*)&last_packet_took_to_sent)[0];
    buf[size++] = ((uint8_t*)&last_packet_took_to_sent)[1];

    float fDegreeCelsius = get_temperature();
    float fHumidity = get_humidity();
    buf[size++] =  static_cast<uint8_t>(round_safe((((isnan(fDegreeCelsius)) ? 0.0f : fDegreeCelsius)) + 20.0f) / 0.3137f);
    buf[size++] =  static_cast<uint8_t>(fHumidity);

    gateway->clear_dispatch_buffer();

    if (server_wifi != NULL)
    {
      server_wifi->send(buf, size);
    }

    if (server_gprs != NULL)
    {
      server_gprs->send(buf, size);
    }

    LOG64_SET("HEARTBEAT: SENT");
    LOG64_NEW_LINE;
  }

}

inline void run_heartbeat()
{
  if (((uint32_t)(((uint32_t)millis()) - heartbeat_last_execute)) >= HEARTBEAT_TIMEOUT)
  {
    heartbeat_last_execute = millis();

    send_message_heartbeat();
  }
}
