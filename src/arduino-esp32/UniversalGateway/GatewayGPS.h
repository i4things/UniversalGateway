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

////////////////////////////////////////////////////////////////////////////
//
//Serial.println(gps.location.lat(), 6); // Latitude in degrees (double)
//Serial.println(gps.location.lng(), 6); // Longitude in degrees (double)
//Serial.print(gps.location.rawLat().negative ? "-" : "+");
//Serial.println(gps.location.rawLat().deg); // Raw latitude in whole degrees
//Serial.println(gps.location.rawLat().billionths);// ... and billionths (u16/u32)
//Serial.print(gps.location.rawLng().negative ? "-" : "+");
//Serial.println(gps.location.rawLng().deg); // Raw longitude in whole degrees
//Serial.println(gps.location.rawLng().billionths);// ... and billionths (u16/u32)
//Serial.println(gps.date.value()); // Raw date in DDMMYY format (u32)
//Serial.println(gps.date.year()); // Year (2000+) (u16)
//Serial.println(gps.date.month()); // Month (1-12) (u8)
//Serial.println(gps.date.day()); // Day (1-31) (u8)
//Serial.println(gps.time.value()); // Raw time in HHMMSSCC format (u32)
//Serial.println(gps.time.hour()); // Hour (0-23) (u8)
//Serial.println(gps.time.minute()); // Minute (0-59) (u8)
//Serial.println(gps.time.second()); // Second (0-59) (u8)
//Serial.println(gps.time.centisecond()); // 100ths of a second (0-99) (u8)
//Serial.println(gps.speed.value()); // Raw speed in 100ths of a knot (i32)
//Serial.println(gps.speed.knots()); // Speed in knots (double)
//Serial.println(gps.speed.mph()); // Speed in miles per hour (double)
//Serial.println(gps.speed.mps()); // Speed in meters per second (double)
//Serial.println(gps.speed.kmph()); // Speed in kilometers per hour (double)
//Serial.println(gps.course.value()); // Raw course in 100ths of a degree (i32)
//Serial.println(gps.course.deg()); // Course in degrees (double)
//Serial.println(gps.altitude.value()); // Raw altitude in centimeters (i32)
//Serial.println(gps.altitude.meters()); // Altitude in meters (double)
//Serial.println(gps.altitude.miles()); // Altitude in miles (double)
//Serial.println(gps.altitude.kilometers()); // Altitude in kilometers (double)
//Serial.println(gps.altitude.feet()); // Altitude in feet (double)
//Serial.println(gps.satellites.value()); // Number of satellites in use (u32)
//Serial.println(gps.hdop.value()); // Horizontal Dim. of Precision (100ths-i32)

#include <TinyGPS++.h>
#include <limits>
#include <sys/time.h>

#define GPS_MIN_SAT_REQUIRED 4
#define GPS_BAUD  9600

// one update every 3 sec
#define GPS_LOCATION_TIMEOUT 3000

HardwareSerial SerialGPS(2);// serial 2

// The TinyGPS++ object
TinyGPSPlus GPS;

#define GPS_TIMEOUT_RUN 10
uint32_t gps_last_run;

#define GPS_MAX_SPEED (200.0d)
#define GPS_MAX_REJECT_COUNT 5
uint8_t gps_reject_count;

bool gps_started;
bool gps_is_time_set;

double gps_lat;
double gps_lng;
uint32_t gps_time;
uint32_t gps_calc_time;
uint16_t gps_satellite_count;
double gps_speed;

double gps_lat_kalman_x;
double gps_lat_kalman_p;
double gps_lat_kalman_q;
double gps_lat_kalman_r;

double gps_lng_kalman_x;
double gps_lng_kalman_p;
double gps_lng_kalman_q;
double gps_lng_kalman_r;

double gps_speed_kalman_x;
double gps_speed_kalman_p;
double gps_speed_kalman_q;
double gps_speed_kalman_r;

//5 sec
#define GPS_SPEED_TIMEOUT 15000
double gps_speed_last_lat;
double gps_speed_last_lng;
uint32_t gps_speed_last_time;
uint32_t gps_speed_calc_time;


///////////////////////////////////////////////////////////////////////////////////
// Helper functions

inline int16_t gps_double2degree(const double & d)
{
  return static_cast<int16_t>(d);
}

inline int32_t gps_double2minsec(const double & d)
{
  int16_t degree =  static_cast<int16_t>(d);
  if (degree >= 0)
  {
    return static_cast<int32_t>((d - degree ) * 1000000000.0d);
  }

  return abs_safe(static_cast<int32_t>((d - degree ) * 1000000000.0d));
}


inline double gps_raw2double (const RawDegrees & raw)
{
  double d = raw.deg + raw.billionths / 1000000000.0d;
  return raw.negative ? -d : d;
}

inline double gps2radian(double d)
{
  return (static_cast<double>(PI) * d) / 180.0d;
}


inline double gps_speed_calc(double newLat,
                             double newLng,
                             double oldLat,
                             double oldLng,
                             uint32_t timeBegin,
                             uint32_t timeFinish)
{
  double latStart = gps2radian(oldLat);
  double lngStart = gps2radian(oldLng);

  double latEnd = gps2radian(newLat);
  double lngEnd = gps2radian(newLng);

  double dist = acos( (sin(latStart) * sin(latEnd)) + (cos(latStart) * cos(latEnd) * cos(lngEnd - lngStart)) ) * 6371000.0d;

  double dist_nm = dist / 1852.0d;

  //  LOG64_SET(F("GPS: TIME ["));
  //  LOG64_SET(timeBegin);
  //  LOG64_SET(timeFinish);
  //  LOG64_SET(F("]"));
  //  LOG64_NEW_LINE;

  return dist_nm / (static_cast<double>(((uint32_t)(timeFinish - timeBegin))) / 3600000.0d);
}

// check if GPS coordinates are valid
inline bool gps_speed_check(double newLat,
                            double newLng,
                            double oldLat,
                            double oldLng,
                            uint32_t timeStart,
                            uint32_t timeEnd)
{
  double speed = gps_speed_calc(newLat, newLng, oldLat, oldLng, timeStart, timeEnd);

  //  LOG64_SET(F("GPS: SPEED ["));
  //  LOG64_SET(String(speed, 2));
  //  LOG64_SET(F("]"));
  //  LOG64_NEW_LINE;


  if (speed > GPS_MAX_SPEED)
  {
    return false;
  }
  return true;

}


///////////////////////////////////////////////////////////////////////////////////
// Kalman filtering
// if x == 0 then assumed not intialized yet
// based on : http://interactive-matter.eu/blog/2009/12/18/filtering-sensor-data-with-a-kalman-filter/
inline void gps_kalman(double & x, double & p, double & q, double & r, double & in_out)
{
  if (x == 0.0f)
  {
    x = in_out;
    return;
  }

  // peform Kalman
  p = p + q;
  double k = p / (p + r);
  x = x + k * (in_out - x);

  p = (1.0f - k) * p;
  in_out =  x;
}

inline void gps_init_time()
{
  gps_time = millis();
  gps_calc_time = millis();

  gps_speed_last_time = millis();
  gps_speed_calc_time = millis();
}

inline void gps_clear()
{
  gps_last_run = millis();

  gps_reject_count = GPS_MAX_REJECT_COUNT; // fill with max to force first valid position to be set

  gps_is_time_set = false;

  gps_lat = NAN;
  gps_lng = NAN;
  gps_speed = 0.0d;

  gps_lat_kalman_x = 0.0d;
  gps_lat_kalman_p = 0.0d;;
  gps_lat_kalman_q = 0.125d;;
  gps_lat_kalman_r = 4.0d;

  gps_lng_kalman_x = 0.0d;
  gps_lng_kalman_p = 0.0d;;
  gps_lng_kalman_q = 0.125d;;
  gps_lng_kalman_r = 4.0d;

  gps_speed_last_lat = 0.0d;
  gps_speed_last_lng = 0.0d;

  gps_init_time();

}

// DEBUG
//String g = String();

inline void run_gps()
{
  if (((uint32_t)(((uint32_t)millis()) - gps_last_run)) >= GPS_TIMEOUT_RUN)
  {
    gps_last_run = millis();

    for (; SerialGPS.available();)
    {
      uint8_t b = SerialGPS.read();

      GPS.encode(b);

      if (!gps_started)
      {
        if ((char)b == '$')
        {
          // OK it seems we do have communication over the serial
          gps_started = true;
          set_monitor_fast_blink();

          LOG64_SET(F("GPS: SERIAL COMMUNICATION OK"));
          LOG64_NEW_LINE;
        }
      }

      //GPS Sentance DEBUG
      //        if ((char)b == '$')
      //        {
      //          LOG64_SET(g);
      //          LOG64_NEW_LINE;
      //          g = "";
      //        }
      //        g += (char)b;

      if (GPS.satellites.isUpdated())
      {
        uint16_t newSatelliteCount = GPS.satellites.value();
        if (gps_satellite_count != newSatelliteCount)
        {
          if ((gps_satellite_count >= GPS_MIN_SAT_REQUIRED) && (newSatelliteCount < GPS_MIN_SAT_REQUIRED))
          {
            // clear speed
            gps_clear();

            set_monitor_fast_blink();

            LOG64_SET(F("GPS: FIX LOST SATELLITES["));
            LOG64_SET(newSatelliteCount);
            LOG64_SET(F("]"));
            LOG64_NEW_LINE;
          }
          else if ((gps_satellite_count < GPS_MIN_SAT_REQUIRED) && (newSatelliteCount >= GPS_MIN_SAT_REQUIRED))
          {
            set_monitor_slow_blink();

            LOG64_SET(F("GPS: FIX ACQUIRED SATELLITES["));
            LOG64_SET(newSatelliteCount);
            LOG64_SET(F("]"));
            LOG64_NEW_LINE;
          }
          else
          {
            //          LOG64_SET(F("GPS: SATELLITES["));
            //          LOG64_SET(newSatelliteCount);
            //          LOG64_SET(F("]"));
            //          LOG64_NEW_LINE;
          }

          gps_satellite_count = newSatelliteCount;
        }
      }

      if (GPS.satellites.value() >= GPS_MIN_SAT_REQUIRED)
      {
        if (GPS.time.isUpdated() || GPS.date.isUpdated())
        {
          if (GPS.time.isValid() && GPS.date.isValid())
          {
            struct tm tm;
            tm.tm_year  = GPS.date.year() - 1900;
            tm.tm_mon   = GPS.date.month() - 1;
            tm.tm_mday  = GPS.date.day();
            tm.tm_hour  = GPS.time.hour();
            tm.tm_min   = GPS.time.minute();
            tm.tm_sec   = GPS.time.second();
            time_t unixtime = mktime(&tm);
            timeval epoch = {unixtime, 0};
            settimeofday((const timeval*)&epoch, 0);

            //          LOG64_SET(F("GPS: TIME SET TO ["));
            //          LOG64_SET((uint32_t)tm.tm_year);
            //          LOG64_SET((uint32_t)tm.tm_mon);
            //          LOG64_SET((uint32_t)tm.tm_mday);
            //          LOG64_SET((uint32_t)tm.tm_hour);
            //          LOG64_SET((uint32_t)tm.tm_min);
            //          LOG64_SET((uint32_t)tm.tm_sec);
            //          LOG64_SET(F("] ESP32 TIME ["));

            //struct timeval tv;
            //gettimeofday(&tv, NULL);

            //          LOG64_SET((uint32_t)tv.tv_sec);
            //          LOG64_SET(F("]"));
            //          LOG64_NEW_LINE;


            gps_is_time_set = true;

            gps_init_time();
          }
        }


        if (GPS.location.isUpdated())
        {
          if (GPS.location.isValid())
          {
            if (((uint32_t)(((uint32_t)millis()) - gps_calc_time)) >= GPS_LOCATION_TIMEOUT)
            {
              gps_calc_time = millis();

              RawDegrees lat = GPS.location.rawLat();
              double newLat = gps_raw2double(lat);

              RawDegrees lng = GPS.location.rawLng();
              double newLng = gps_raw2double(lng);

              bool reject = !gps_speed_check(newLat, newLng, gps_lat, gps_lng, gps_time, millis());

              if ((gps_reject_count < GPS_MAX_REJECT_COUNT) && reject)
              {
                LOG64_SET(F("GPS: NEW POSITION REJECTED ["));
                LOG64_SET(String(newLat, 6));
                LOG64_SET(String(newLng, 6));
                LOG64_SET(F("]"));
                LOG64_NEW_LINE;

                gps_reject_count++;
              }
              else
              {
                if (gps_reject_count >= GPS_MAX_REJECT_COUNT)
                {
                  LOG64_SET(F("GPS: MAX REJECTED COUNT HIT ["));
                  LOG64_SET(gps_reject_count);
                  LOG64_SET(String(newLat, 6));
                  LOG64_SET(String(newLng, 6));
                  LOG64_SET(F("]"));
                  LOG64_NEW_LINE;
                }

                gps_reject_count = 0;

                //          LOG64_SET(F("GPS: LAT BEFORE KALMAN["));
                //          LOG64_SET(String(newLat, 6));
                //          LOG64_SET(F("]"));
                //          LOG64_NEW_LINE;
                gps_kalman(gps_lat_kalman_x,
                           gps_lat_kalman_p,
                           gps_lat_kalman_q,
                           gps_lat_kalman_r,
                           newLat);
                //          LOG64_SET(F("GPS: LAT AFTER KALMAN["));
                //          LOG64_SET(String(newLat, 6));
                //          LOG64_SET(F("]"));
                //          LOG64_NEW_LINE;

                //          LOG64_SET(F("GPS: LNG BEFORE KALMAN["));
                //          LOG64_SET(String(newLng, 6));
                //          LOG64_SET(F("]"));
                //          LOG64_NEW_LINE;
                gps_kalman(gps_lng_kalman_x,
                           gps_lng_kalman_p,
                           gps_lng_kalman_q,
                           gps_lng_kalman_r,
                           newLng);
                //          LOG64_SET(F("GPS: LNG AFTER KALMAN["));
                //          LOG64_SET(String(newLng, 6));
                //          LOG64_SET(F("]"));
                //          LOG64_NEW_LINE;

                if ((newLat != gps_lat) || (newLng != gps_lng))
                {

                  gps_lat = newLat;
                  gps_lng = newLng;
                  gps_time = millis();

                  //                LOG64_SET(F("GPS: NEW POSITION ["));
                  //                LOG64_SET(String(newLat, 6));
                  //                LOG64_SET(String(newLng, 6));
                  //                LOG64_SET(F("]"));
                  //                LOG64_NEW_LINE;

                  // speed calculation

                  if (((uint32_t)(((uint32_t)millis()) - gps_speed_calc_time)) >= GPS_SPEED_TIMEOUT)
                  {
                    gps_speed_calc_time = millis();

                    if ((gps_speed_last_lat == 0.0d) || (gps_speed_last_lng == 0.0d) || (gps_lat == 0.0d) || (gps_lng == 0.0d))
                    {
                      gps_speed_last_lat = gps_lat;
                      gps_speed_last_lng = gps_lng;
                      gps_speed_last_time = millis();
                    }
                    else
                    {
                      double newSpeed = gps_speed_calc(gps_lat, gps_lng, gps_speed_last_lat, gps_speed_last_lng , gps_speed_last_time,  millis());

                      if (newSpeed <= GPS_MAX_SPEED)
                      {
                        gps_speed_last_lat = gps_lat;
                        gps_speed_last_lng = gps_lng;
                        gps_speed_last_time = millis();

                        gps_speed = newSpeed;

                        LOG64_SET(F("GPS: NEW SPEED ["));
                        LOG64_SET(String(gps_speed, 2));
                        LOG64_SET(F("]"));
                        LOG64_NEW_LINE;

                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

inline void init_gps()
{
  gps_started = false;

  gps_clear();

  SerialGPS.begin(GPS_BAUD, SERIAL_8N1, GPS_PIN_RX, GPS_PIN_TX);

}

inline double get_gps_speed()
{
  return 12.3f;
  //return gps_speed;
}

inline uint8_t get_gps_satellite_count()
{
  // no need sync get/set on 8 bits only
  return 9;
  //return gps_satellite_count;
}

inline  double get_gps_lat()
{
  return 47.801323d;
  //return gps_lat;
}

inline  double get_gps_lng()
{
  return 11.8184533d;
  //return  gps_lng;
}

inline uint32_t get_gps_time()
{
  if (gps_is_time_set)
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    LOG64_SET(F("GPS: TIME REQUESTED["));
    LOG64_SET((uint32_t)tv.tv_sec);
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;

    return tv.tv_sec;
  }

  return 0;
}
