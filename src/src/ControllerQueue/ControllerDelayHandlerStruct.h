#ifndef CONTROLLERQUEUE_CONTROLLER_DELAY_HANDLER_STRUCT_H
#define CONTROLLERQUEUE_CONTROLLER_DELAY_HANDLER_STRUCT_H

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/TimingStats.h"
#include "../DataStructs/UnitMessageCount.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/CPlugins.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/Protocol.h"
#include "../Helpers/_CPlugin_Helper.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Networking.h"
#include "../Helpers/Scheduler.h"
#include "../Helpers/StringConverter.h"

#include <Arduino.h>
#include <list>
#include <memory> // For std::shared_ptr
#include <new>    // std::nothrow

#ifndef CONTROLLER_QUEUE_MINIMAL_EXPIRE_TIME
  #define CONTROLLER_QUEUE_MINIMAL_EXPIRE_TIME 10000
#endif

/*********************************************************************************************\
* ControllerDelayHandlerStruct
\*********************************************************************************************/
template<class T>
struct ControllerDelayHandlerStruct {
  ControllerDelayHandlerStruct() :
    lastSend(0),
    minTimeBetweenMessages(CONTROLLER_DELAY_QUEUE_DELAY_DFLT),
    expire_timeout(0),
    max_queue_depth(CONTROLLER_DELAY_QUEUE_DEPTH_DFLT),
    attempt(0),
    max_retries(CONTROLLER_DELAY_QUEUE_RETRY_DFLT),
    delete_oldest(false),
    must_check_reply(false),
    deduplicate(false) {}

  void configureControllerSettings(const ControllerSettingsStruct& settings) {
    minTimeBetweenMessages = settings.MinimalTimeBetweenMessages;
    max_queue_depth        = settings.MaxQueueDepth;
    max_retries            = settings.MaxRetry;
    delete_oldest          = settings.DeleteOldest;
    must_check_reply       = settings.MustCheckReply;
    deduplicate            = settings.deduplicate();
    if (settings.allowExpire()) {
      expire_timeout = max_queue_depth * max_retries * (minTimeBetweenMessages + settings.ClientTimeout);
      if (expire_timeout < CONTROLLER_QUEUE_MINIMAL_EXPIRE_TIME) {
        expire_timeout = CONTROLLER_QUEUE_MINIMAL_EXPIRE_TIME;
      }
    } else {
      expire_timeout = 0;
    }

    // Set some sound limits when not configured
    if (max_queue_depth == 0) { max_queue_depth = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT; }

    if (max_retries == 0) { max_retries = CONTROLLER_DELAY_QUEUE_RETRY_DFLT; }

    if (minTimeBetweenMessages == 0) { minTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT; }

    // No less than 10 msec between messages.
    if (minTimeBetweenMessages < 10) { minTimeBetweenMessages = 10; }
  }

  bool readyToProcess(const T& element) const {
    const protocolIndex_t protocolIndex = getProtocolIndex_from_ControllerIndex(element.controller_idx);
    if (protocolIndex == INVALID_PROTOCOL_INDEX) {
      return false;
    }
    if (Protocol[protocolIndex].needsNetwork) {
      return NetworkConnected(10);
    }
    return true;
  }

  bool queueFull(const T& element) const {
    if (sendQueue.size() >= max_queue_depth) { return true; }

    // Number of elements is not exceeding the limit, check memory
    int freeHeap = FreeMem();
    {
      #ifdef USE_SECOND_HEAP
      const int freeHeap2 = FreeMem2ndHeap();
      if (freeHeap2 < freeHeap) {
        freeHeap = freeHeap2;
      }
      #endif
    }

    if (freeHeap > 5000) { 
      return false; // Memory is not an issue.
    }
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("Controller-");
      log += element.controller_idx + 1;
      log += F(" : Memory used: ");
      log += getQueueMemorySize();
      log += F(" bytes ");
      log += sendQueue.size();
      log += F(" items ");
      log += freeHeap;
      log += F(" free");
      addLog(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG
    return true;
  }

  // Return true if message is already present in the queue
  bool isDuplicate(const T& element) const {
    // Some controllers may receive duplicate messages, due to lost acknowledgement
    // This is actually the same message, so this should not be processed.
    if (!unitLastMessageCount.isNew(element.getUnitMessageCount())) {
      return true;
    }
    // The unit message count is still stored to make sure a new one with the same count
    // is considered a duplicate, even when the queue is empty.
    unitLastMessageCount.add(element.getUnitMessageCount());

    // the setting 'deduplicate' does look at the content of the message and only compares it to messages in the queue.
    if (deduplicate && !sendQueue.empty()) {
      // Use reverse iterator here, as it is more likely a duplicate is added shortly after another.
      auto it = sendQueue.rbegin(); // Same as back()
      for (; it != sendQueue.rend(); ++it) {
        if (element.isDuplicate(*it)) {
#ifndef BUILD_NO_DEBUG
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            const cpluginID_t cpluginID = getCPluginID_from_ControllerIndex(it->controller_idx);
            String log = get_formatted_Controller_number(cpluginID);
            log += F(" : Remove duplicate");
            addLog(LOG_LEVEL_DEBUG, log);
          }
#endif // ifndef BUILD_NO_DEBUG
          return true;
        }
      }
    }
    return false;
  }

  // Try to add to the queue, if permitted by "delete_oldest"
  // Return true when item was added, or skipped as it was considered a duplicate
  bool addToQueue(T&& element) {
    if (isDuplicate(element)) {
      return true;
    }

    if (delete_oldest) {
      // Force add to the queue.
      // If max buffer is reached, the oldest in the queue (first to be served) will be removed.
      while (queueFull(element)) {
        sendQueue.pop_front();
        attempt = 0;
      }
    }

    if (!queueFull(element)) {
      #ifdef USE_SECOND_HEAP
      HeapSelectIram ephemeral;
      sendQueue.push_back(element);
      #else
      sendQueue.push_back(std::move(element));
      #endif

      return true;
    }
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      const cpluginID_t cpluginID = getCPluginID_from_ControllerIndex(element.controller_idx);
      String log = get_formatted_Controller_number(cpluginID);
      log += " : queue full";
      addLog(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG
    return false;
  }

  // Get the next element.
  // Remove front element when max_retries is reached.
  T* getNext() {
    if (sendQueue.empty()) { return nullptr; }

    if (attempt > max_retries) {
      sendQueue.pop_front();
      attempt = 0;
    }

    if (expire_timeout != 0) {
      bool done = false;
      while (!done && !sendQueue.empty()) {
        if (timePassedSince(sendQueue.front()._timestamp) < static_cast<long>(expire_timeout)) {
          done = true;
        } else {
          sendQueue.pop_front();
          attempt = 0;
        }
      }
    }

    if (sendQueue.empty()) { return nullptr; }
    return &sendQueue.front();
  }

  // Mark as processed and return time to schedule for next process.
  // Return 0 when nothing to process.
  // @param remove_from_queue indicates whether the elements should be removed from the queue.
  unsigned long markProcessed(bool remove_from_queue) {
    if (sendQueue.empty()) { return 0; }

    if (remove_from_queue) {
      sendQueue.pop_front();
      attempt = 0;
      lastSend = millis();
    } else {
      ++attempt;
    }
    return getNextScheduleTime();
  }

  unsigned long getNextScheduleTime() const {
    if (sendQueue.empty()) { return 0; }
    unsigned long nextTime = lastSend + minTimeBetweenMessages;

    if (timePassedSince(nextTime) > 0) {
      nextTime = millis();
    }

    if (nextTime == 0) { nextTime = 1; // Just to make sure it will be executed
    }
    return nextTime;
  }

  // Set the "lastSend" to "now" + some additional delay.
  // This will cause the next schedule time to be delayed to 
  // msecFromNow + minTimeBetweenMessages
  void setAdditionalDelay(unsigned long msecFromNow) {
    lastSend = millis() + msecFromNow;
  }

  size_t getQueueMemorySize() const {
    size_t totalSize = 0;

    for (auto it = sendQueue.begin(); it != sendQueue.end(); ++it) {
      totalSize += it->getSize();
    }
    return totalSize;
  }

  std::list<T>  sendQueue;
  mutable UnitLastMessageCount_map unitLastMessageCount;
  unsigned long lastSend;
  unsigned int  minTimeBetweenMessages;
  unsigned long expire_timeout = 0;
  uint8_t          max_queue_depth;
  uint8_t          attempt;
  uint8_t          max_retries;
  bool          delete_oldest;
  bool          must_check_reply;
  bool          deduplicate;
};



// Uncrustify must not be used on macros, so turn it off.
// Also make sure to wrap the forward declaration of this function in the same wrappers 
// as it may not split the forward declaration into multiple lines.
//
// *INDENT-OFF*



// Define the function wrappers to handle the calling to Cxxx_DelayHandler etc.
// If someone knows how to add leading zeros in macros, please be my guest :)


// This macro defines the code needed to create the 'process_c##NNN####M##_delay_queue()'
// function and all needed objects and forward declarations.
// It is a macro to prevent common typo errors.
// This function will perform the (re)scheduling and mark if it is processed (and can be removed)
// The controller itself must implement the 'do_process_c004_delay_queue' function to actually
// send the data.
// Its return value must state whether it can be marked 'Processed'.
// N.B. some controllers only can send one value per iteration, so a returned "false" can mean it
//      was still successful. The controller should keep track of the last value sent
//      in the element stored in the queue.
#define DEFINE_Cxxx_DELAY_QUEUE_MACRO(NNN, M)                                                                          \
  bool do_process_c##NNN####M##_delay_queue(int controller_number,                                                     \
                                           const C##NNN####M##_queue_element & element,                                \
                                           ControllerSettingsStruct & ControllerSettings);                             \
  typedef ControllerDelayHandlerStruct<C##NNN####M##_queue_element> C##NNN####M##_DelayHandler_t;                      \
  extern C##NNN####M##_DelayHandler_t *C##NNN####M##_DelayHandler;                                                     \
  void process_c##NNN####M##_delay_queue();                                                                            \
  bool init_c##NNN####M##_delay_queue(controllerIndex_t ControllerIndex);                                              \
  void exit_c##NNN####M##_delay_queue();                                                                               \

#define DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(NNN, M)                                                                      \
  C##NNN####M##_DelayHandler_t *C##NNN####M##_DelayHandler = nullptr;                                                  \
  void process_c##NNN####M##_delay_queue() {                                                                           \
    if (C##NNN####M##_DelayHandler == nullptr) return;                                                                 \
    C##NNN####M##_queue_element *element(C##NNN####M##_DelayHandler->getNext());                                       \
    if (element == nullptr) return;                                                                                       \
    MakeControllerSettings(ControllerSettings);                                                                        \
    bool ready = true;                                                                                                 \
    if (!AllocatedControllerSettings()) {                                                                              \
      ready = false;                                                                                                   \
    } else {                                                                                                           \
      LoadControllerSettings(element->controller_idx, ControllerSettings);                                             \
      C##NNN####M##_DelayHandler->configureControllerSettings(ControllerSettings);                                     \
      if (!C##NNN####M##_DelayHandler->readyToProcess(*element)) { ready = false; }                                    \
    }                                                                                                                  \
    if (ready) {                                                                                                       \
      START_TIMER;                                                                                                     \
      C##NNN####M##_DelayHandler->markProcessed(do_process_c##NNN####M##_delay_queue(M, *element, ControllerSettings)); \
      STOP_TIMER(C##NNN####M##_DELAY_QUEUE);                                                                           \
    }                                                                                                                  \
    Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C##NNN####M##_DELAY_QUEUE, C##NNN####M##_DelayHandler->getNextScheduleTime());         \
  }                                                                                                                    \
  bool init_c##NNN####M##_delay_queue(controllerIndex_t ControllerIndex) {                                             \
    if (C##NNN####M##_DelayHandler == nullptr) {                                                                       \
      C##NNN####M##_DelayHandler = new (std::nothrow) (C##NNN####M##_DelayHandler_t);                                  \
    }                                                                                                                  \
    if (C##NNN####M##_DelayHandler == nullptr) { return false; }                                                       \
    MakeControllerSettings(ControllerSettings);                                                                        \
    if (!AllocatedControllerSettings()) {                                                                              \
      return false;                                                                                                    \
    }                                                                                                                  \
    LoadControllerSettings(ControllerIndex, ControllerSettings);                                                       \
    C##NNN####M##_DelayHandler->configureControllerSettings(ControllerSettings);                                       \
    return true;                                                                                                       \
  }                                                                                                                    \
  void exit_c##NNN####M##_delay_queue() {                                                                              \
    if (C##NNN####M##_DelayHandler != nullptr) {                                                                       \
      delete C##NNN####M##_DelayHandler;                                                                               \
      C##NNN####M##_DelayHandler = nullptr;                                                                            \
    }                                                                                                                  \
  }                                                                                                                    \




// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*


#endif // CONTROLLERQUEUE_CONTROLLER_DELAY_HANDLER_STRUCT_H