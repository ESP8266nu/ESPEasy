#ifndef DATASTRUCTS_TIMINGSTATS_H
#define DATASTRUCTS_TIMINGSTATS_H

#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../../ESPEasy_common.h"


#ifdef USES_TIMING_STATS

#include "../Helpers/ESPEasy_time_calc.h"

# include <Arduino.h>
# include <map>


/*********************************************************************************************\
* TimingStats
\*********************************************************************************************/


# define LOADFILE_STATS            0
# define SAVEFILE_STATS            1
# define LOOP_STATS                2
# define PLUGIN_CALL_50PS          3
# define PLUGIN_CALL_10PS          4
# define PLUGIN_CALL_10PSU         5
# define PLUGIN_CALL_1PS           6
# define SENSOR_SEND_TASK          7
# define CPLUGIN_CALL_10PS         8
# define CPLUGIN_CALL_50PS         9
# define SEND_DATA_STATS           10
# define COMPUTE_FORMULA_STATS     11
# define PROC_SYS_TIMER            12
# define SET_NEW_TIMER             13
# define TIME_DIFF_COMPUTE         14
# define MQTT_DELAY_QUEUE          15
# define C001_DELAY_QUEUE          16
# define C002_DELAY_QUEUE          17
# define C003_DELAY_QUEUE          18
# define C004_DELAY_QUEUE          19
# define C005_DELAY_QUEUE          20
# define C006_DELAY_QUEUE          21
# define C007_DELAY_QUEUE          22
# define C008_DELAY_QUEUE          23
# define C009_DELAY_QUEUE          24
# define C010_DELAY_QUEUE          25
# define C011_DELAY_QUEUE          26
# define C012_DELAY_QUEUE          27
# define C013_DELAY_QUEUE          28
# define C014_DELAY_QUEUE          29
# define C015_DELAY_QUEUE          30
# define C016_DELAY_QUEUE          31
# define C017_DELAY_QUEUE          32
# define C018_DELAY_QUEUE          33
# define C019_DELAY_QUEUE          34
# define C020_DELAY_QUEUE          35
# define C021_DELAY_QUEUE          36
# define C022_DELAY_QUEUE          37
# define C023_DELAY_QUEUE          38
# define C024_DELAY_QUEUE          39
# define C025_DELAY_QUEUE          40
# define C018_AIR_TIME             41
# define TRY_CONNECT_HOST_TCP      42
# define TRY_CONNECT_HOST_UDP      43
# define HOST_BY_NAME_STATS        44
# define CONNECT_CLIENT_STATS      45
# define LOAD_CUSTOM_TASK_STATS    46
# define WIFI_ISCONNECTED_STATS    47
# define WIFI_NOTCONNECTED_STATS   48
# define LOAD_TASK_SETTINGS        49
# define TRY_OPEN_FILE             50
# define FS_GC_SUCCESS             51
# define FS_GC_FAIL                52
# define PARSE_SYSVAR              53
# define PARSE_SYSVAR_NOCHANGE     54
# define PARSE_TEMPLATE_PADDED     55
# define RULES_PROCESSING          56
# define GRAT_ARP_STATS            57
# define SAVE_TO_RTC               58
# define BACKGROUND_TASKS          59
# define HANDLE_SCHEDULER_IDLE     60
# define HANDLE_SCHEDULER_TASK     61
# define HANDLE_SERVING_WEBPAGE    62
# define HANDLE_ESPEASY_NOW_LOOP   63
# define EXPIRED_ESPEASY_NOW_LOOP  64
# define INVALID_ESPEASY_NOW_LOOP  65
# define RECEIVE_ESPEASY_NOW_LOOP  66
# define ESPEASY_NOW_SEND_MSG_SUC  67
# define ESPEASY_NOW_SEND_MSG_FAIL 68
# define ESPEASY_NOW_SEND_PCKT     69
# define ESPEASY_NOW_DEDUP_LOOP    70
# define WIFI_SCAN_ASYNC           71
# define WIFI_SCAN_SYNC            72



class TimingStats {
public:

  TimingStats();

  void         add(unsigned long time);
  void         reset();
  bool         isEmpty() const;
  float        getAvg() const;
  unsigned int getMinMax(unsigned long& minVal,
                         unsigned long& maxVal) const;
  bool         thresholdExceeded(unsigned long threshold) const;

private:

  float _timeTotal;
  unsigned int _count;
  unsigned long _maxVal;
  unsigned long _minVal;
};


const __FlashStringHelper * getPluginFunctionName(int function);
bool   mustLogFunction(int function);
const __FlashStringHelper * getCPluginCFunctionName(CPlugin::Function function);
bool   mustLogCFunction(CPlugin::Function function);
String getMiscStatsName(int stat);


extern std::map<int, TimingStats> pluginStats;
extern std::map<int, TimingStats> controllerStats;
extern std::map<int, TimingStats> miscStats;
extern unsigned long timingstats_last_reset;

# define START_TIMER const unsigned long statisticsTimerStart(micros());
# define STOP_TIMER_TASK(T, F) \
  if (mustLogFunction(F)) pluginStats[(T) * 256 + (F)].add(usecPassedSince(statisticsTimerStart));
# define STOP_TIMER_CONTROLLER(T, F) \
  if (mustLogCFunction(F)) controllerStats[(T) * 256 + static_cast<int>(F)].add(usecPassedSince(statisticsTimerStart));

// #define STOP_TIMER_LOADFILE miscStats[LOADFILE_STATS].add(usecPassedSince(statisticsTimerStart));
# define STOP_TIMER(L) if (Settings.EnableTimingStats()) { miscStats[L].add(usecPassedSince(statisticsTimerStart)); }

// Add a timer statistic value in usec.
# define ADD_TIMER_STAT(L, T) if (Settings.EnableTimingStats()) { miscStats[L].add(T); }

#else // ifdef USES_TIMING_STATS

# define START_TIMER ;
# define STOP_TIMER_TASK(T, F) ;
# define STOP_TIMER_CONTROLLER(T, F) ;
# define STOP_TIMER(L) ;
# define ADD_TIMER_STAT(L, T) ;


// FIXME TD-er: This class is used as a parameter in functions defined in .ino files.
// The Arduino build process tries to forward declare all functions it can find, regardless of defines.
// Meaning we must make sure the forward declaration of the TimingStats class is made, since it is used as an argument in some function.
class TimingStats;

#endif // ifdef USES_TIMING_STATS

#endif // DATASTRUCTS_TIMINGSTATS_H
