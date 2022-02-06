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
// LED monitoring

#define MONITOR_TIMEOUT_RUN 250
uint32_t monitor_lst_run;

uint8_t MONITOR_STATE;
uint8_t MONITOR_BLINK_STATE;

inline void run_monitor()
{
  if (((uint32_t)(((uint32_t)millis()) - monitor_lst_run)) >= MONITOR_TIMEOUT_RUN)
  {
    monitor_lst_run = millis();

    switch (MONITOR_BLINK_STATE)
    {
      case 0 : // always ON
        {
          MONITOR_STATE = 1;
          digitalWrite(MONITOR_PIN, HIGH);
        }
        break;
      case 1 : // fast blink
        {
          if (MONITOR_STATE == 1)
          {
            MONITOR_STATE = 0;
            digitalWrite(MONITOR_PIN, LOW);
          }
          else
          {
            MONITOR_STATE = 1;
            digitalWrite(MONITOR_PIN, HIGH);
          }
        }
        break;
      case 2 : // LONG BLINK STATE 1 ( off and wait one timeout )
        {
          MONITOR_BLINK_STATE = 3;
          MONITOR_STATE = 0;
          digitalWrite(MONITOR_PIN, LOW);
        }
        break;
      case 3: // 1x timeout
      case 4: // 2x timeout
      case 5: // 3x timeout
      case 6: // 4x timeou
      case 7: // 5x timeou
      case 8: // 6x timeout
        {
          MONITOR_BLINK_STATE += 1;
        }
        break;
      case 9: // 5x timeout and ON one timeout
        {
          MONITOR_BLINK_STATE = 2;
          MONITOR_STATE = 1;
          digitalWrite(MONITOR_PIN, HIGH);
        }
    }
  }
}

inline void set_monitor_fast_blink()
{
  MONITOR_BLINK_STATE = 1;
}

inline void set_monitor_slow_blink()
{
  MONITOR_BLINK_STATE = 2;
}

inline void set_monitor_on_blink()
{
  MONITOR_BLINK_STATE = 0;
}

inline void init_monitor()
{

  LOG64_SET(F("MONITOR: INIT:"));
  LOG64_NEW_LINE;

  MONITOR_STATE = 1;
  MONITOR_BLINK_STATE = 0;

  pinMode(MONITOR_PIN, OUTPUT);
  digitalWrite(MONITOR_PIN, HIGH);

}
