#include "../Helpers/_Internal_GPIO_pulseHelper.h"


#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../WebServer/Markup_Forms.h"

#include "../../ESPEasy_common.h"

#include <Arduino.h>

#define GPIO_PLUGIN_ID  1


const __FlashStringHelper * Internal_GPIO_pulseHelper::toString(GPIOtriggerMode mode)
{
  switch (mode) {
    case GPIOtriggerMode::None: return F("None");
    case GPIOtriggerMode::Change: return F("Change");
    case GPIOtriggerMode::Rising: return F("Rising");
    case GPIOtriggerMode::Falling: return F("Falling");
    case GPIOtriggerMode::PulseLow: return F("PULSE Low");
    case GPIOtriggerMode::PulseHigh: return F("PULSE High");
    case GPIOtriggerMode::PulseChange: return F("PULSE Change");
  }
  return F("");
}

void Internal_GPIO_pulseHelper::addGPIOtriggerMode(const __FlashStringHelper *label,
                                                   const __FlashStringHelper *id,
                                                   GPIOtriggerMode            currentSelection)
{
  #define NR_TRIGGER_MODES  7
  const __FlashStringHelper *options[NR_TRIGGER_MODES];
  const int optionValues[NR_TRIGGER_MODES] = {
    static_cast<int>(GPIOtriggerMode::None),
    static_cast<int>(GPIOtriggerMode::Change),
    static_cast<int>(GPIOtriggerMode::Rising),
    static_cast<int>(GPIOtriggerMode::Falling),
    static_cast<int>(GPIOtriggerMode::PulseLow),
    static_cast<int>(GPIOtriggerMode::PulseHigh),
    static_cast<int>(GPIOtriggerMode::PulseChange)
  };

  for (int i = 0; i < NR_TRIGGER_MODES; ++i) {
    options[i] = Internal_GPIO_pulseHelper::toString(static_cast<Internal_GPIO_pulseHelper::GPIOtriggerMode>(optionValues[i]));
  }
  addFormSelector(label, id, NR_TRIGGER_MODES, options, optionValues, static_cast<int>(currentSelection));
}

Internal_GPIO_pulseHelper::Internal_GPIO_pulseHelper(Internal_GPIO_pulseHelper::pulseCounterConfig configuration)
  : config(configuration) {}

Internal_GPIO_pulseHelper::~Internal_GPIO_pulseHelper() {
  detachInterrupt(digitalPinToInterrupt(config.gpio));
}

bool Internal_GPIO_pulseHelper::init()
{
  if (checkValidPortRange(GPIO_PLUGIN_ID, config.gpio)) {
    pinMode(config.gpio, config.pullupPinMode);

    pulseModeData.currentStableState = config.interruptPinMode == GPIOtriggerMode::PulseLow ? HIGH : LOW;
    pulseModeData.lastCheckState     = HIGH;

    // initialize internal variables for PULSE mode handling
    #ifdef PULSE_STATISTIC
    resetStatsErrorVars();
    ISRdata.Step0counter         = ISRdata.pulseTotalCounter;
    pulseModeData.Step1counter   = ISRdata.pulseTotalCounter;
    pulseModeData.Step2OKcounter = ISRdata.pulseTotalCounter;
    pulseModeData.Step3OKcounter = ISRdata.pulseTotalCounter;
    #endif

    const int intPinMode = static_cast<int>(config.interruptPinMode) & MODE_INTERRUPT_MASK;
    attachInterruptArg(
      digitalPinToInterrupt(config.gpio),
      reinterpret_cast<void (*)(void *)>(ISR_pulseCheck),
      this, intPinMode);

    return true;
  }
  return false;
}

void Internal_GPIO_pulseHelper::getPulseCounters(unsigned long& pulseCounter, unsigned long& pulseTotalCounter, float& pulseTime_msec)
{
  pulseCounter      = ISRdata.pulseCounter;
  pulseTotalCounter = ISRdata.pulseTotalCounter;
  pulseTime_msec    = static_cast<float>(ISRdata.pulseTime) / 1000.0f;
}

void Internal_GPIO_pulseHelper::setPulseCountTotal(unsigned long pulseTotalCounter)
{
  ISRdata.pulseTotalCounter = pulseTotalCounter;
}

void Internal_GPIO_pulseHelper::setPulseCounter(unsigned long pulseCounter, float pulseTime_msec)
{
  ISRdata.pulseCounter = pulseCounter;
  ISRdata.pulseTime    = static_cast<uint64_t>(pulseTime_msec * 1000.0f);
}

void Internal_GPIO_pulseHelper::resetPulseCounter()
{
  ISRdata.pulseCounter = 0;
  ISRdata.pulseTime    = 0;
}

void Internal_GPIO_pulseHelper::doPulseStepProcessing(int pStep)
{
  switch (pStep)
  {
    case GPIO_PULSE_HELPER_PROCESSING_STEP_0:
      // regularily called to check if the trigger has flagged the next signal edge
    {
      if (ISRdata.initStepsFlags)
      {
        // schedule step 1 in remaining milliseconds from debounce time
        long delayTime =
          static_cast<long>(
            (static_cast<int64_t>(config.debounceTime_micros)) -
            static_cast<int64_t>(us_Since_triggerTimestamp())
            ) / 1000L;

        if (delayTime < 0)
        {
          // if debounce time was too short or we were called too late by Scheduler
          #ifdef PULSE_STATISTIC
          pulseModeData.Step0ODcounter++; // count occurences
          pulseModeData.StepOverdueMax[pStep] = max(pulseModeData.StepOverdueMax[pStep], -delayTime);
          #endif // PULSE_STATISTIC
          delayTime = 0;
        }
        Scheduler.setPluginTaskTimer(delayTime, config.taskIndex, GPIO_PULSE_HELPER_PROCESSING_STEP_1);

        #ifdef PULSE_STATISTIC

        // FIXME TD-er: Why only correct this when statistics are collected?
        ISRdata.triggerTimestamp = getMicros64() + delayTime * 1000L;
        #endif // PULSE_STATISTIC

        // initialization done
        ISRdata.initStepsFlags = false;
      }
      break;
    }

    case GPIO_PULSE_HELPER_PROCESSING_STEP_1: // read pin status
    {
      #ifdef PULSE_STATISTIC
      pulseModeData.Step1counter++;
      pulseModeData.setStepOverdueMax(pStep, msec_Since_triggerTimestamp());
      #endif // PULSE_STATISTIC

      //  read current state from this tasks's GPIO
      pulseModeData.lastCheckState = digitalRead(config.gpio);

      // after debounceTime/2, do step 2
      Scheduler.setPluginTaskTimer(config.debounceTime >> 1, config.taskIndex, GPIO_PULSE_HELPER_PROCESSING_STEP_2);

      #ifdef PULSE_STATISTIC

      // FIXME TD-er: Why only correct this when statistics are collected?
      ISRdata.triggerTimestamp = getMicros64() + (config.debounceTime >> 1) * 1000L;
      #endif // PULSE_STATISTIC
      break;
    }

    case GPIO_PULSE_HELPER_PROCESSING_STEP_2: // 1st validation of pin status
    {
      #ifdef PULSE_STATISTIC
      pulseModeData.setStepOverdueMax(pStep, msec_Since_triggerTimestamp());
      #endif // PULSE_STATISTIC

      //  read current state from this tasks's GPIO
      const int pinState = digitalRead(config.gpio);

      if (pinState == pulseModeData.lastCheckState)

      // we found stable state
      {
        #ifdef PULSE_STATISTIC
        pulseModeData.Step2OKcounter++;
        #endif // PULSE_STATISTIC
        // after debounceTime/2, do step 3
        Scheduler.setPluginTaskTimer(config.debounceTime >> 1, config.taskIndex, GPIO_PULSE_HELPER_PROCESSING_STEP_3);
      }
      else

      // we found unexpected different pin state
      {
        #ifdef PULSE_STATISTIC
        pulseModeData.Step2NOKcounter++;
        #endif // PULSE_STATISTIC
        // lets ignore previous pin status. It might have been a spike. Try to detect stable signal
        pulseModeData.lastCheckState = pinState;    // now trust the new state
        // after debounceTime/2, do step 2 again
        Scheduler.setPluginTaskTimer(config.debounceTime >> 1, config.taskIndex, GPIO_PULSE_HELPER_PROCESSING_STEP_2);
      }

      #ifdef PULSE_STATISTIC

      // FIXME TD-er: Why only correct this when statistics are collected?
      ISRdata.triggerTimestamp = getMicros64() + (config.debounceTime >> 1) * 1000L;
      #endif // PULSE_STATISTIC
      break;
    }

    case GPIO_PULSE_HELPER_PROCESSING_STEP_3: // 2nd validation of pin status and counting
    {
      #ifdef PULSE_STATISTIC
      pulseModeData.setStepOverdueMax(pStep, msec_Since_triggerTimestamp());
      #endif // PULSE_STATISTIC

      // determine earliest effective start time of current stable pulse (= NOW - 2 * DebounceTime )
      const uint64_t pulseChangeTime = getMicros64() - (config.debounceTime_micros << 1);

      // determine how long the current stable pulse was lasting
      if (pulseModeData.currentStableState == HIGH) // pulse was HIGH
      {
        pulseModeData.pulseHighTime = pulseChangeTime - ISRdata.currentStableStartTime;
      }
      else // pulse was LOW
      {
        pulseModeData.pulseLowTime = pulseChangeTime - ISRdata.currentStableStartTime;
      }

      //  read current state from this tasks's GPIO
      const int pinState = digitalRead(config.gpio);

      if (pinState == pulseModeData.lastCheckState)

      // we found the same state as in step 2. It is stable and valid.
      {
        processStablePulse(pinState, pulseChangeTime);
      }
      else

      // we found unexpected different pin state
      {
        #ifdef PULSE_STATISTIC
        pulseModeData.Step3NOKcounter++;
        #endif // PULSE_STATISTIC

        // ignore spike from previous step. It is regarded as spike within previous=current signal. Again try to detect stable signal
        pulseModeData.lastCheckState = pinState;    // now trust the previous=new state
        Scheduler.setPluginTaskTimer(config.debounceTime >> 1, config.taskIndex, GPIO_PULSE_HELPER_PROCESSING_STEP_2);

        #ifdef PULSE_STATISTIC

        // FIXME TD-er: Why only correct this when statistics are collected?
        ISRdata.triggerTimestamp = getMicros64() + (config.debounceTime >> 1) * 1000L;
        #endif // PULSE_STATISTIC
      }
      break;
    }
    default:
    {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log; log.reserve(48);
        log = F("_P003:PLUGIN_TIMER_IN: Invalid processingStep: "); log += pStep;
        addLog(LOG_LEVEL_ERROR, log);
      }
      break;
    }
  }
}

uint64_t Internal_GPIO_pulseHelper::us_Since_triggerTimestamp() const {
  return getMicros64() - ISRdata.triggerTimestamp;
}

long Internal_GPIO_pulseHelper::msec_Since_triggerTimestamp() const {
  return static_cast<long>(us_Since_triggerTimestamp()) / 1000L;
}

/*********************************************************************************************\
*  Processing for found stable pulse
\*********************************************************************************************/
void Internal_GPIO_pulseHelper::processStablePulse(int pinState, uint64_t pulseChangeTime)
{
  if (pinState != pulseModeData.currentStableState)

  // The state changed. Previous sable pulse ends, new starts
  {
    #ifdef PULSE_STATISTIC
    pulseModeData.Step3OKcounter++;
    #endif // PULSE_STATISTIC

    // lets terminate the previous pulse and setup start point for new stable one
    pulseModeData.currentStableState = !pulseModeData.currentStableState;
    ISRdata.currentStableStartTime   = pulseChangeTime;

    // now provide the counter result values for the ended pulse ( depending on mode type)
    switch (config.interruptPinMode)
    {
      case GPIOtriggerMode::PulseChange:
      {
        if (pulseModeData.currentStableState == LOW) { // HIGH had ended
          ISRdata.pulseTime = pulseModeData.pulseHighTime;
        }
        else {                                         // LOW has ended
          ISRdata.pulseTime = pulseModeData.pulseLowTime;
        }

        ISRdata.pulseCounter++;
        ISRdata.pulseTotalCounter++;
        break;
      }
      case GPIOtriggerMode::PulseHigh:
      {
        if (pulseModeData.currentStableState == LOW) // HIGH had ended (else do nothing)
        {
          ISRdata.pulseTime = pulseModeData.pulseLowTime + pulseModeData.pulseHighTime;
          ISRdata.pulseCounter++;
          ISRdata.pulseTotalCounter++;
        }
        break;
      }
      case GPIOtriggerMode::PulseLow:
      {
        if (pulseModeData.currentStableState == HIGH) // LOW had ended (else do nothing)
        {
          ISRdata.pulseTime = pulseModeData.pulseLowTime + pulseModeData.pulseHighTime;
          ISRdata.pulseCounter++;
          ISRdata.pulseTotalCounter++;
        }
        break;
      }
      default:
      {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log;
          log.reserve(48);
          log  = F("_P003:PLUGIN_TIMER_IN: Invalid modeType: ");
          log += static_cast<int>(config.interruptPinMode);
          addLog(LOG_LEVEL_ERROR, log);
        }
        break;
      }
    }
  }
  else

  // we found the same stable state as before
  {
    #ifdef PULSE_STATISTIC
    pulseModeData.Step3IGNcounter++;
    #endif // PULSE_STATISTIC
    // do nothing. Ignore interupt. previous stable state was confirmed probably after a spike
  }

  #ifdef PULSE_STATISTIC
  doStatisticLogging(pulseModeData.StatsLogLevel);
  #endif // PULSE_STATISTIC

  // allow next pulse check call from interrupt
  ISRdata.processingFlags = false;
}

void IRAM_ATTR Internal_GPIO_pulseHelper::ISR_pulseCheck(Internal_GPIO_pulseHelper *self)
{
  noInterrupts(); // s0170071: avoid nested interrups due to bouncing.

  if (self->config.useEdgeMode())
  {
    // legacy edge Mode types
    // KP: we use here P003_currentStableStartTime[taskID] to persist the PulseTime (pulseTimePrevious)
    const uint64_t currentTime          = getMicros64();
    const uint64_t timeSinceLastTrigger = currentTime -
                                          self->ISRdata.currentStableStartTime;

    if (timeSinceLastTrigger > self->config.debounceTime_micros) // check with debounce time for this task
    {
      self->ISRdata.pulseCounter++;
      self->ISRdata.pulseTotalCounter++;
      self->ISRdata.pulseTime              = timeSinceLastTrigger;
      self->ISRdata.currentStableStartTime = currentTime; // reset when counted to determine interval between counted pulses
    }
  }
  else

  // processing for new PULSE mode types
  {
    #ifdef PULSE_STATISTIC
    self->ISRdata.Step0counter++;
    #endif // PULSE_STATISTIC

    // check if processing is allowed (not blocked) for this task (taskID)
    if (!self->ISRdata.processingFlags)
    {
      // initiate processing
      self->ISRdata.processingFlags  = true; // block further initiations as long as async processing is taking place
      self->ISRdata.initStepsFlags   = true; // PLUGIN_FIFTY_PER_SECOND is polling for this flag set
      self->ISRdata.triggerTimestamp = getMicros64();
    }
  }
  interrupts(); // enable interrupts again.
}

#ifdef PULSE_STATISTIC

void Internal_GPIO_pulseHelper::updateStatisticalCounters(int par1) {
  ISRdata.Step0counter         -= ISRdata.pulseTotalCounter - par1;
  pulseModeData.Step1counter   -= ISRdata.pulseTotalCounter - par1;
  pulseModeData.Step2OKcounter -= ISRdata.pulseTotalCounter - par1;
  pulseModeData.Step3OKcounter -= ISRdata.pulseTotalCounter - par1;
}

void Internal_GPIO_pulseHelper::setStatsLogLevel(uint8_t logLevel) {
  pulseModeData.StatsLogLevel = logLevel;
}

/*********************************************************************************************\
*  reset statistical error cunters and overview variables
\*********************************************************************************************/
void Internal_GPIO_pulseHelper::resetStatsErrorVars() {
  // initialize statistical step counters from TotalCounter and error counters with 0
  pulseModeData.Step2NOKcounter = 0;
  pulseModeData.Step3NOKcounter = 0;
  pulseModeData.Step3IGNcounter = 0;
  pulseModeData.Step0ODcounter  = 0;

  for (int pStep = 0; pStep <= P003_PSTEP_MAX; pStep++) {
    pulseModeData.StepOverdueMax[pStep] = 0;
  }
}

/*********************************************************************************************\
*  write statistic counters to logfile
\*********************************************************************************************/
void Internal_GPIO_pulseHelper::doStatisticLogging(uint8_t logLevel)
{
  if (loglevelActiveFor(logLevel)) {
    // Statistic to logfile. E.g: ... [123/1|111|100/5|80/3/4|40] [12243|3244]
    String log; log.reserve(125);
    log  = F("Pulse:");
    log += F("Stats (GPIO) [step0|1|2|3|tot(ok/nok/ign)] [lo|hi]= (");
    log += config.gpio;                       log += F(") [");
    log += ISRdata.Step0counter;              log += '|';
    log += pulseModeData.Step1counter;        log += '|';
    log += pulseModeData.Step2OKcounter;      log += '/';
    log += pulseModeData.Step2NOKcounter;     log += '|';
    log += pulseModeData.Step3OKcounter;      log += '/';
    log += pulseModeData.Step3NOKcounter;     log += '/';
    log += pulseModeData.Step3IGNcounter;     log += '|';
    log += ISRdata.pulseTotalCounter;         log += F("] [");
    log += pulseModeData.pulseLowTime / 1000L;  log += '|';
    log += pulseModeData.pulseHighTime / 1000L; log += ']';
    addLog(logLevel, log);
  }
}

/*********************************************************************************************\
*  write collected timing values to logfile
\*********************************************************************************************/
void Internal_GPIO_pulseHelper::doTimingLogging(uint8_t logLevel)
{
  if (loglevelActiveFor(logLevel)) {
    // Timer to logfile. E.g: ... [4|12000|13444|12243|3244]
    String log;
    log.reserve(120);
    log  = F("Pulse:");
    log += F("OverDueStats (GPIO) [dbTim] {step0OdCnt} [maxOdTimeStep0|1|2|3]= (");
    log += config.gpio;  log += F(") [");
    log += config.debounceTime;  log += F("] {");
    log += pulseModeData.Step0ODcounter;  log += F("} [");

    for (int pStep = 0; pStep <= P003_PSTEP_MAX; pStep++) {
      log += pulseModeData.StepOverdueMax[pStep];

      if (pStep < P003_PSTEP_MAX) { log += '|'; }
    }
    log += ']';
    addLog(logLevel, log);
  }
}

#endif // PULSE_STATISTIC
