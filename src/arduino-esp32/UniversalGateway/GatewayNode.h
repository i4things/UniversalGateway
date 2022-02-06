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
// Embedded node

uint32_t node_last_execute;

inline void init_node()
{
  node_last_execute = millis();

  LOG64_SET(F("NODE: INIT"));
  LOG64_NEW_LINE;
}

inline uint32_t last_send()
{
   return  ((uint32_t)(((uint32_t)millis()) - node_last_execute));
}

inline uint32_t next_send()
{
   return (GetSetupReportTimeout() - ((uint32_t)(((uint32_t)millis()) - node_last_execute)));
}

inline void send_message_node()
{
  // gen packetsize = 0;
  uint8_t buf[IoTThing_MAX_MESSAGE_LEN];
  uint8_t size = 0;

  uint32_t gps_time = get_gps_time();
  // gps time will return 0 when not set - e.g. omitted value
  buf[size++] = ((uint8_t*)&gps_time)[0];
  buf[size++] = ((uint8_t*)&gps_time)[1];
  buf[size++] = ((uint8_t*)&gps_time)[2];
  buf[size++] = ((uint8_t*)&gps_time)[3];

  buf[size++] = 0; // soil temp
  buf[size++] = 0; // soil moisture

  float fDegreeCelsius = get_temperature();
  float fHumidity = get_humidity();
  buf[size++] =  static_cast<uint8_t>(round_safe((((isnan(fDegreeCelsius)) ? 0.0f : fDegreeCelsius)) + 20.0f) / 0.3137f);
  buf[size++] =  static_cast<uint8_t>(fHumidity);

  buf[size++] = 0;// PM

  LOG64_SET(F("NODE: TIME["));
  LOG64_SET(String(gps_time));
  LOG64_SET(F("] T["));
  LOG64_SET(String(fDegreeCelsius, 1));
  LOG64_SET(F("] H["));
  LOG64_SET(String(fHumidity, 0));
  LOG64_SET(F("%]"));

  double lat = get_gps_lat();
  double lng = get_gps_lng();

  if ((!isnan(lat)) && (!isnan(lng)))
  {
    int16_t deg_lat = gps_double2degree(lat);
    int32_t min_sec_lat = gps_double2minsec(lat);
    int16_t deg_lng = gps_double2degree(lat);
    int32_t min_sec_lng = gps_double2minsec(lat);

    buf[size++] = ((uint8_t*)&deg_lat)[0];
    buf[size++] = ((uint8_t*)&deg_lat)[1];

    buf[size++] = ((uint8_t*)&min_sec_lat)[0];
    buf[size++] = ((uint8_t*)&min_sec_lat)[1];
    buf[size++] = ((uint8_t*)&min_sec_lat)[2];
    buf[size++] = ((uint8_t*)&min_sec_lat)[3];

    buf[size++] = ((uint8_t*)&deg_lng)[0];
    buf[size++] = ((uint8_t*)&deg_lng)[1];

    buf[size++] = ((uint8_t*)&min_sec_lng)[0];
    buf[size++] = ((uint8_t*)&min_sec_lng)[1];
    buf[size++] = ((uint8_t*)&min_sec_lng)[2];
    buf[size++] = ((uint8_t*)&min_sec_lng)[3];

    LOG64_SET(F("LAT["));
    LOG64_SET(deg_lat);
    LOG64_SET(min_sec_lat);
    LOG64_SET(F(" ] LON["));
    LOG64_SET(deg_lng);
    LOG64_SET(min_sec_lng);
    LOG64_SET(F("]"));
  }

  LOG64_NEW_LINE;

  if (thing_embedded != NULL)
  {
    if (thing_embedded->is_ready())
    {
      LOG64_SET(F("NODE: PREV MESSAGE DISCARDED"));
      LOG64_NEW_LINE;
      thing_embedded->cancel();
    }

    if (thing_embedded->send(buf, size))
    {
      LOG64_SET(F("NODE: MESSAGE SEND TO GATEWAY"));
      LOG64_NEW_LINE;
    }
    else
    {
      LOG64_SET(F("NODE: MESSAGE FORMAT WRONG"));
      LOG64_NEW_LINE;
    }
  }


}

inline void run_node()
{
  if (((uint32_t)(((uint32_t)millis()) - node_last_execute)) >= GetSetupReportTimeout())
  {
    node_last_execute = millis();

    send_message_node();
  }
}
