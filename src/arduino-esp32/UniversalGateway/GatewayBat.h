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
// Li Bat Voltmeter

#include <Wire.h>

#define BAT_TIMEOUT  1000
uint32_t bat_last_execute;
#define BAT_AVERAGE_TIMEOUT  10000
uint32_t bat_average_last_execute;

//https://esp-idf.readthedocs.io/en/latest/api-reference/peripherals/adc.html#example-of-reading-calibrated-values
#define ESP32_DEFAULT_REF 1100

#include "driver/adc.h"
#include "esp_adc_cal.h"

esp_adc_cal_characteristics_t bat_adc_chars;


// 1 /  ((2 * 3.3) / 1023)
#define BAT_DIVIDER 155.0f
#define BAT_MAX_COUNT 10
float  bat_storage[BAT_MAX_COUNT];
uint8_t  bat_storage_counter;

float bat_value;

float bat_kalman_x;
float bat_kalman_p;
float bat_kalman_q;
float bat_kalman_r;

///////////////////////////////////////////////////////////////////////////////////
// Helper functions

inline void bat_insert_sort(float arr[], float val, uint8_t count)
{
  uint8_t idx = 0;

  //Find the place to insert the element
  for (; (idx < count) && (arr[idx] <= val); idx++)
  {
  }

  //Move backward elements
  for (uint8_t j = count; j > idx; j--)
  {
    arr[j] = arr[j - 1];
  }

  //Insert element
  arr[idx] = val;
}

///////////////////////////////////////////////////////////////////////////////////
// Kalman filtering
// if x == 0 then assumed not intialized yet
// based on : http://interactive-matter.eu/blog/2009/12/18/filtering-sensor-data-with-a-kalman-filter/

inline void bat_kalman(float & x, float & p, float & q, float & r, uint16_t & in_out)
{
  if (x == 0.0f)
  {
    x = in_out;
    return;
  }

  // peform Kalman
  p = p + q;
  float k = p / (p + r);
  x = x + k * (((float)in_out) - x);

  p = (1.0f - k) * p;
  in_out =  (uint16_t)x;
}

inline float  bat_calc(uint16_t from_sensor)
{
  // perform kalman
  bat_kalman(bat_kalman_x,
             bat_kalman_p,
             bat_kalman_q,
             bat_kalman_r,
             from_sensor);

  from_sensor = (uint16_t) esp_adc_cal_raw_to_voltage(from_sensor, &bat_adc_chars);

  return ((float)from_sensor) / BAT_DIVIDER;
}

inline void init_bat()
{

  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(BAT_PIN, ADC_ATTEN_11db);
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, ESP32_DEFAULT_REF, &bat_adc_chars);
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
  {
    LOG64_SET(F("BAT: Characterized using Two Point Value."));
    LOG64_NEW_LINE;
  }
  else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
  {
    LOG64_SET(F("BAT: Characterized using eFuse Vref."));
    LOG64_NEW_LINE;
  }
  else
  {
    LOG64_SET(F("BAT: Characterized using Default Vref."));
    LOG64_NEW_LINE;
  }

  bat_last_execute = millis();
  bat_average_last_execute = millis();

  bat_storage_counter = 0;

  bat_value = NAN;

  bat_kalman_x = 0.0f;
  bat_kalman_p = 0.0f;
  bat_kalman_q = 0.125f;
  bat_kalman_r = 4.0f;

  LOG64_SET(F("BAT: INIT"));
  LOG64_NEW_LINE;
}

inline void run_bat()
{
  if (((uint32_t)(((uint32_t)millis()) - bat_last_execute)) >= BAT_TIMEOUT)
  {
    bat_last_execute = millis();

    if (bat_storage_counter < BAT_MAX_COUNT)
    {
      uint16_t value = (uint16_t)analogRead(BAT_PIN);

      //      LOG64_SET(F("BAT: VALUE["));
      //      LOG64_SET(value);
      //      LOG64_SET(F("]"));
      //      LOG64_NEW_LINE;

      bat_insert_sort(bat_storage,  bat_calc(value), bat_storage_counter++);
    }
  }

  if (((uint32_t)(((uint32_t)millis()) - bat_average_last_execute)) >= BAT_AVERAGE_TIMEOUT)
  {
    bat_average_last_execute = millis();

    if (bat_storage_counter > 0)
    {
      if (!isnan(bat_value))
      {
        bat_value = (bat_value + bat_storage[bat_storage_counter / 2]) / 2;
      }
      else
      {
        bat_value = bat_storage[bat_storage_counter / 2];
      }

      //      LOG64_SET(F("BAT_V: VALUE["));
      //      LOG64_SET(bat_value);
      //      LOG64_SET(F("]"));
      //      LOG64_NEW_LINE;

      bat_storage_counter = 0;

    }
  }
}

inline float get_bat()
{
  //return bat_value;
  return ((float)random(3, 4)) + (((float)random(33, 99)) / 100.0f);
}
