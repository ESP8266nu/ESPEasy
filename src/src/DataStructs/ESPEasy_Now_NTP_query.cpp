#include "ESPEasy_Now_NTP_query.h"

#ifdef USES_ESPEASY_NOW


// Typical time wander for ESP nodes is 0.04 ms/sec
// Meaning per 25 sec, the time may wander 1 msec.
# define TIME_WANDER_FACTOR  25000

// Max time it may take to get a reply.
# define MAX_DELAY_FOR_REPLY  5000

# define MINIMUM_TIME_BETWEEN_UPDATES  (1800 * 1000) // Half an hour

# include "../ESPEasyCore/ESPEasy_Log.h"
# include "../Globals/ESPEasy_time.h"
# include "../Helpers/ESPEasy_time_calc.h"
# include "../../ESPEasy_fdwdecl.h"

ESPEasy_Now_NTP_query::ESPEasy_Now_NTP_query()
{
  reset(true);
}

bool ESPEasy_Now_NTP_query::getMac(MAC_address& mac) const
{
  if (_timeSource == timeSource_t::No_time_source) { return false; }

  mac.set(_mac);
  return true;
}

// Only use peers if there is no external source available.
// A network without external synced source may drift as a whole
// All nodes in the network may be in sync with each other, but get out of sync with the rest of the world.
// Therefore use a strong bias for external synced nodes.
// But also must make sure the same NTP synced node will be held responsible for the entire network.
unsigned long ESPEasy_Now_NTP_query::computeExpectedWander(timeSource_t  timeSource,
                                                           unsigned long timePassedSinceLastTimeSync)
{
  unsigned long expectedWander_ms = timePassedSinceLastTimeSync / TIME_WANDER_FACTOR;

  switch (timeSource) {
    case timeSource_t::GPS_PPS_time_source:
    {
      expectedWander_ms += 1;
      break;
    }
    case timeSource_t::GPS_time_source:
    {
      expectedWander_ms += 10;
      break;
    }
    case timeSource_t::NTP_time_source:
    {
      expectedWander_ms += 10;
      break;
    }

    case  timeSource_t::ESP_now_peer:
    {
      expectedWander_ms += 100;
      break;
    }

    case timeSource_t::Restore_RTC_time_source:
    {
      expectedWander_ms += 2000;
      break;
    }
    case timeSource_t::Manual_set:
    {
      expectedWander_ms += 10000;
      break;
    }
    case timeSource_t::No_time_source:
    {
      // Cannot sync from it.
      return 1 << 30;
    }
  }
  return expectedWander_ms;
}

void ESPEasy_Now_NTP_query::find_best_NTP(const MAC_address& mac,
                                          timeSource_t       timeSource,
                                          unsigned long      timePassedSinceLastTimeSync)
{
  if (_millis_out != 0) {
    if (timePassedSince(_millis_out) > MAX_DELAY_FOR_REPLY) {
      reset(false);

      // A query was sent, but didn't get a reply in time.
    } else {
      // A query is still pending, so don't add new possible best NTP.
      return;
    }
  }
  bool updated                    = false;
  unsigned long expectedWander_ms =
    computeExpectedWander(timeSource, timePassedSinceLastTimeSync);

  // First check if it matches the current best candidate.
  bool matches_current_best = mac == _mac;

  if (matches_current_best) {
    // Update expected wander based on current time since last sync
    updated = true;
  } else {
    if (_expectedWander_ms > expectedWander_ms) {
      // We found a good new candidate
      bool matches_prev_fail = mac == _mac_prev_fail;

      if (matches_prev_fail) {
        // No need to retry.
        return;
      }
      mac.get(_mac);
      updated = true;
    }
  }

  if (updated) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(64);
      log += F(ESPEASY_NOW_NAME);
      log += F(": Best NTP peer: ");
      log += MAC_address(_mac).toString();
      log += F(" Wander ");
      log += _expectedWander_ms;
      log += F(" ms -> ");
      log += expectedWander_ms;
      log += F(" ms");
      addLog(LOG_LEVEL_INFO, log);
    }
    _timeSource        = timeSource;
    _expectedWander_ms = expectedWander_ms;
  }
}

void ESPEasy_Now_NTP_query::reset(bool success)
{
  // FIXME TD-er: For now also clear failed MAC. Have to think of a mechanism for multiple failed attempts
  success            = true;
  _unixTime_d        = 0.0;
  _millis_in         = 0;
  _millis_out        = 0;
  _expectedWander_ms = (1 << 30);
  _timeSource        = timeSource_t::No_time_source;

  for (uint8_t i = 0; i < 6; ++i) {
    _mac_prev_fail[i] = success ? 0 : _mac[i];
  }

  for (uint8_t i = 0; i < 6; ++i) {
    _mac[i] = 0;
  }
}

bool ESPEasy_Now_NTP_query::hasLowerWander() const
{
  const long timePassed                  = timePassedSince(node_time.lastSyncTime);
  unsigned long currentExpectedWander_ms = computeExpectedWander(node_time.timeSource, timePassed);

  if (node_time.timeSource < _timeSource) {
    // Current time source is of better category than the best other.
    // So only update from the other if ours was too long ago.
    if (currentExpectedWander_ms < 1000) { return false; }
  }

  if (node_time.timeSource == _timeSource) {
    // Same time source category.
    // Try to limit the number of time updates by only accepting updates if time since last sync is over N seconds
    if (timePassed < MINIMUM_TIME_BETWEEN_UPDATES) { return false; }
  }

  return _expectedWander_ms < currentExpectedWander_ms;
}

bool ESPEasy_Now_NTP_query::isBroadcast() const
{
  for (uint8_t i = 0; i < 6; ++i) {
    if (_mac[i] != 0xFF) { return false; }
  }
  return true;
}

void ESPEasy_Now_NTP_query::markSendTime()
{
  _millis_out = millis();
}

void ESPEasy_Now_NTP_query::createBroadcastNTP()
{
  for (uint8_t i = 0; i < 6; ++i)
  {
    _mac[i] = 0xFF;
  }
  createReply(0);
}

void ESPEasy_Now_NTP_query::createReply(unsigned long queryReceiveTimestamp)
{
  _millis_in = queryReceiveTimestamp;
  node_time.now();
  _timeSource        = node_time.timeSource;
  _unixTime_d        = node_time.sysTime;
  _expectedWander_ms = computeExpectedWander(node_time.timeSource, timePassedSince(node_time.lastSyncTime));

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(64);
    log += F(ESPEASY_NOW_NAME);
    log += F(": Create NTP reply to: ");
    log += MAC_address(_mac).toString();
    log += F(" Wander ");
    log += _expectedWander_ms;
    log += F(" ms");
    addLog(LOG_LEVEL_INFO, log);
  }
  markSendTime();
}

bool ESPEasy_Now_NTP_query::processReply(const ESPEasy_Now_NTP_query& received, unsigned long receiveTimestamp)
{
  if (!received.hasLowerWander()) { return false; }

  bool isBroadcast = received.isBroadcast();
  long air_time    = 0;

  if (isBroadcast) {
    // Average air time for an ESP-now message is roughly 1 msec.
    air_time = 1;
  } else {
    // Not a broadcast NTP update, so we must have initiated it ourselves.
    // Check how long it took to get a reply.
    const long timediff_source = timeDiff(_millis_out, receiveTimestamp);

    if (timediff_source > MAX_DELAY_FOR_REPLY) {
      // Took too long, discard it.
      reset(false);
      return false;
    }

    const long timediff_reply = timeDiff(received._millis_in, received._millis_out);
    air_time = (timediff_source - timediff_reply) / 2;
  }
  double compensation_ms = air_time + timePassedSince(receiveTimestamp);
  double new_unixTime_d  = received._unixTime_d + (compensation_ms / 1000);
  node_time.setExternalTimeSource(new_unixTime_d, timeSource_t::ESP_now_peer);
  node_time.now();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(64);
    log  = F(ESPEASY_NOW_NAME);
    log += F(": NTP air time: ");
    log += air_time;
    log += F(" Compensation: ");
    log += String(compensation_ms, 1);
    log += F(" ms");
    addLog(LOG_LEVEL_INFO, log);
  }
  reset(true);
  return true;
}

#endif // ifdef USES_ESPEASY_NOW
