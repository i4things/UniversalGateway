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
// DHT ( Humidity/Temp)

#include "DHT.h"

#define DHT_TYPE DHT22

#define DHT_TIMEOUT_RUN 30000
uint32_t dht_last_run;

DHT dht(DHT_PIN, DHT_TYPE);

float dht_temp;
float dht_humidity;

inline void run_dht()
{
  if (((uint32_t)(((uint32_t)millis()) - dht_last_run)) >= DHT_TIMEOUT_RUN)
  {
    dht_last_run = millis(); 
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if ((isnan(humidity)) || (isnan(temperature)))
    {
      LOG64_SET(F("DHT: READ FAILED"));
      LOG64_NEW_LINE;
    }
    else
    {
      dht_humidity = humidity;
      dht_temp = temperature;
    }

    LOG64_SET(F("DHT: T["));
    LOG64_SET(String(dht_temp, 1));
    LOG64_SET(F("] H["));
    LOG64_SET(dht_humidity);
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;
  }
}

inline void init_dht()
{
  dht_last_run = millis();
  //important make sure DHT is initialized
  dht.begin();

  dht_temp = NAN;
  dht_humidity = NAN;

}

inline float get_humidity()
{
  return 68.0f;
  //return dht_humidity;
}

inline float get_temperature()
{
  return 12.3f;
  //return dht_temp;
}
