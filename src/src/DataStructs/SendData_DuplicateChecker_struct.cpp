#include "SendData_DuplicateChecker_struct.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Plugins.h"
#include "../Helpers/CRC_functions.h"
#include "../Helpers/ESPEasy_time_calc.h"


#define HISTORIC_ELEMENT_LIFETIME  10000 // 10 seconds

const uint32_t SendData_DuplicateChecker_struct::DUPLICATE_CHECKER_INVALID_KEY = 0;

uint32_t create_compare_key(taskIndex_t taskIndex, const String& compare_key)
{
  pluginID_t id = getPluginID_from_TaskIndex(taskIndex);

  if (id == INVALID_PLUGIN_ID) {
    return SendData_DuplicateChecker_struct::DUPLICATE_CHECKER_INVALID_KEY;
  }
  uint32_t key = calc_CRC32(reinterpret_cast<const uint8_t *>(compare_key.c_str()), compare_key.length());
  key += id;

  // consider the 0 as invalid key, so never return 0 on a valid key
  if (key == SendData_DuplicateChecker_struct::DUPLICATE_CHECKER_INVALID_KEY) {
    ++key;
  }
  return key;
}

uint32_t SendData_DuplicateChecker_struct::add(struct EventStruct *event, const String& compare_key)
{
  uint32_t key = create_compare_key(event->TaskIndex, compare_key);

  if (key != DUPLICATE_CHECKER_INVALID_KEY) {
    if (historicKey(key)) {
      // Item already exists in the queue, no need to ask around
      return DUPLICATE_CHECKER_INVALID_KEY;
    }
    {
      _queue_mutex.lock();
      _queue.emplace(std::make_pair(key, SendData_DuplicateChecker_data(event)));
      _queue_mutex.unlock();
    }
    {
      _historic_mutex.lock();
      _historic[key] = millis();
      _historic_mutex.unlock();
    }
  }
  return key;
}

bool SendData_DuplicateChecker_struct::historicKey(uint32_t key)
{
  // Consider invalid key always as historic key so it will never be processed.
  if (key == DUPLICATE_CHECKER_INVALID_KEY) { return true; }
  auto it = _historic.find(key);

  if (it == _historic.end())
  {
    // Someone asked about it, so mark it here
    _historic_mutex.lock();
    _historic[key] = millis();
    _historic_mutex.unlock();
    return false;
  }

  // Apparently we've seen another instance of that message, renew the last seen timestamp
  it->second = millis();
  return true;
}

void SendData_DuplicateChecker_struct::remove(uint32_t key)
{
  auto it = _queue.find(key);

  if (it != _queue.end()) {
    #ifndef BUILD_NO_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLog(LOG_LEVEL_DEBUG, F(ESPEASY_NOW_NAME ": message not sent as processed elsewhere"));
    }
    #endif
    {
      _queue_mutex.lock();
      _queue.erase(it);
      _queue_mutex.unlock();
    }
  }
}

void SendData_DuplicateChecker_struct::loop()
{
  purge_old_historic();

  for (auto it = _queue.begin(); it != _queue.end();) {
    if (it->second.doSend()) {
      _queue_mutex.lock();
      _queue.erase(it);
      _queue_mutex.unlock();

      // Processed one, others will be processed later
      return;
    } else {
      ++it;
    }
  }
}

void SendData_DuplicateChecker_struct::purge_old_historic()
{
  for (auto it = _historic.begin(); it != _historic.end();)
  {
    if (timePassedSince(it->second) > HISTORIC_ELEMENT_LIFETIME) {
      _historic_mutex.lock();
      it = _historic.erase(it);
      _historic_mutex.unlock();
    } else {
      ++it;
    }
  }
}
