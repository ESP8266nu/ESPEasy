#ifndef HELPERS_ESPEASY_TIME_H
#define HELPERS_ESPEASY_TIME_H

#include <Arduino.h>

#include "../DataTypes/ESPEasyTimeSource.h"




class ESPEasy_time {
public:

  ESPEasy_time();

  struct tm   addSeconds(const struct tm& ts,
                         int              seconds,
                         bool             toLocalTime) const;
  static void breakTime(unsigned long timeInput,
                        struct tm   & tm);

  // Restore the last known system time
  // This may be useful to get some idea of what time it is.
  // This way the unit can do things based on local time even when NTP servers may not respond.
  // Do not use this when booting from deep sleep.
  // Only call this once during boot.
  void restoreLastKnownUnixTime(unsigned long lastSysTime, uint8_t deepSleepState);

  void          setExternalTimeSource(double       time,
                                      timeSource_t source);

  uint32_t      getUnixTime() const;

  void          initTime();

  // Update and get the current systime
  unsigned long now();

  // Update time and return whether the minute has changed since last check.
  bool          reportNewMinute();

  bool          systemTimePresent() const;

  bool          getNtpTime(double& unixTime_d);


  /********************************************************************************************\
     Date/Time string formatters
   \*********************************************************************************************/

public:


// Format the current Date separated by the given delimiter
// Default date format example: 20161231 (YYYYMMDD)
String getDateString(char delimiter = '\0') const;

// Format given Date separated by the given delimiter
// date format example with '-' delimiter: 2016-12-31 (YYYY-MM-DD)
static String getDateString(const struct tm& ts, char delimiter);

// Formats the current Time
// Default time format example: 235959 (HHMMSS)
String getTimeString(char delimiter = '\0', bool show_seconds=true) const;

String getTimeString_ampm(char delimiter = '\0', bool show_seconds=true) const;

// returns the current Time separated by the given delimiter
// time format example with ':' delimiter: 23:59:59 (HH:MM:SS)
static String getTimeString(const struct tm& ts, char delimiter, bool am_pm, bool show_seconds);




String getDateTimeString(char dateDelimiter = '-', char timeDelimiter = ':',  char dateTimeDelimiter = ' ') const;
String getDateTimeString_ampm(char dateDelimiter = '-', char timeDelimiter = ':',  char dateTimeDelimiter = ' ') const;

// returns the current Date and Time separated by the given delimiter
// if called like this: getDateTimeString('\0', '\0', '\0');
// it will give back this: 20161231235959  (YYYYMMDDHHMMSS)
static String getDateTimeString(const struct tm& ts, char dateDelimiter = '-', char timeDelimiter = ':',  char dateTimeDelimiter = ' ', bool am_pm = false);


/********************************************************************************************\
   Get current time/date
 \*********************************************************************************************/

// Get the year given a Unix time stamp
static int year(unsigned long t);

// Get the weekday, given a Unix time stamp
static int weekday(unsigned long t);

// Convert a weekday number (Sun = 1 ... Sat = 7) to a 3 letter string
static String weekday_str(int wday); 


// Get current year.
int year() const 
{
  return 1900 + tm.tm_year;
}

// Get current month
uint8_t month() const 
{
  return tm.tm_mon + 1; // tm_mon starts at 0
}

// Get current day of the month
uint8_t day() const 
{
  return tm.tm_mday;
}

// Get current hour
uint8_t hour() const 
{
  return tm.tm_hour;
}

// Get current minute
uint8_t minute() const 
{
  return tm.tm_min;
}

// Get current second
uint8_t second() const 
{
  return tm.tm_sec;
}

// day of week, sunday is day 1
int weekday() const 
{
  return tm.tm_wday;
}

String weekday_str() const;






/********************************************************************************************\
   Sunrise/Sunset calculations
 \*********************************************************************************************/

public:

  // Compute the offset in seconds of the substring +/-<nn>[smh]
  static int getSecOffset(const String& format);
  String     getSunriseTimeString(char delimiter) const;
  String     getSunsetTimeString(char delimiter) const;
  String     getSunriseTimeString(char delimiter,
                                  int  secOffset) const;
  String     getSunsetTimeString(char delimiter,
                                 int  secOffset) const;

private:

  static float sunDeclination(int doy);
  static float diurnalArc(float dec,
                          float lat);
  static float equationOfTime(int doy);
  static int   dayOfYear(int year,
                         int month,
                         int day);

  void      calcSunRiseAndSet();
  struct tm getSunRise(int secOffset) const;
  struct tm getSunSet(int secOffset) const;

public:

  struct tm tm;
  uint32_t syncInterval = 3600; // time sync will be attempted after this many seconds
  double sysTime = 0.0;         // Use high resolution double to get better sync between nodes when using NTP
  uint32_t prevMillis = 0;
  uint32_t nextSyncTime = 0;
  uint32_t lastSyncTime = 0;
  uint32_t lastNTPSyncTime = 0;
  double externalUnixTime_d = -1.0; // Used to set time from a source other than NTP.
  struct tm tsRise, tsSet;
  struct tm sunRise;
  struct tm sunSet;
  timeSource_t timeSource = timeSource_t::No_time_source;

  uint8_t PrevMinutes = 0;
};


#endif // HELPERS_ESPEASY_TIME_H
