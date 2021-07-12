#include "../PluginStructs/P098_data_struct.h"

#ifdef USES_P098

# include "../ESPEasyCore/ESPEasyGPIO.h"

# include "../Commands/GPIO.h" // FIXME TD-er: Only needed till we can call GPIO commands from the ESPEasy core.

# define GPIO_PLUGIN_ID  1


const __FlashStringHelper * P098_config_struct::toString(P098_config_struct::PWM_mode_type PWM_mode) {
  switch (PWM_mode) {
    case P098_config_struct::PWM_mode_type::NoPWM:  return F("No PWM");
    case P098_config_struct::PWM_mode_type::PWM:    return F("PWM");

    case P098_config_struct::PWM_mode_type::MAX_TYPE: break;
  }

  return F("");
}

P098_data_struct::P098_data_struct(const P098_config_struct& config) : _config(config) {}

P098_data_struct::~P098_data_struct() {
  if (initialized) {
    detachInterrupt(digitalPinToInterrupt(_config.limitA.gpio));
    detachInterrupt(digitalPinToInterrupt(_config.limitB.gpio));
    detachInterrupt(digitalPinToInterrupt(_config.encoder.gpio));
  }
}

bool P098_data_struct::begin(int pos, int limitApos, int limitBpos)
{
  if (!initialized) {
    initialized = true;

    const bool switchPosSet = pos != 0 && limitApos != 0;

    limitA.switchposSet = switchPosSet;
    limitA.switchpos    = switchPosSet ? limitApos : 0;
    limitB.switchpos    = limitBpos;
    position            = pos + limitA.switchpos;
    stop();
    state = P098_data_struct::State::Idle;

    if (setPinMode(_config.limitA)) {
      attachInterruptArg(
        digitalPinToInterrupt(_config.limitA.gpio),
        reinterpret_cast<void (*)(void *)>(ISRlimitA),
        this, CHANGE);
    }

    if (setPinMode(_config.limitB)) {
      attachInterruptArg(
        digitalPinToInterrupt(_config.limitB.gpio),
        reinterpret_cast<void (*)(void *)>(ISRlimitB),
        this, CHANGE);
    }

    if (setPinMode(_config.encoder)) {
      attachInterruptArg(
        digitalPinToInterrupt(_config.encoder.gpio),
        reinterpret_cast<void (*)(void *)>(ISRencoder),
        this, CHANGE); // Act on 'CHANGE', not on rising/falling
    }
  }

  return true;
}

bool P098_data_struct::loop()
{
  const State old_state(state);

  check_limit_switch(_config.limitA, limitA);
  check_limit_switch(_config.limitB, limitB);

  switch (state) {
    case P098_data_struct::State::Idle:
      return true;
    case P098_data_struct::State::RunFwd:
    {
      checkLimit(limitB);
      break;
    }
    case P098_data_struct::State::RunRev:
    {
      checkLimit(limitA);
      break;
    }
    case P098_data_struct::State::StopLimitSw:
    case P098_data_struct::State::StopPosReached:
      // Still in a state that needs inspection
      return false;
  }

  // Must check when state has changed from running to some other state
  return old_state == state;
}

bool P098_data_struct::homePosSet() const
{
  return limitA.switchposSet;
}

bool P098_data_struct::canRun()
{
  if (!homePosSet()) { return false; }

  switch (state) {
    case P098_data_struct::State::Idle:
    case P098_data_struct::State::RunFwd:
    case P098_data_struct::State::RunRev:
      return true;
    case P098_data_struct::State::StopLimitSw:
    case P098_data_struct::State::StopPosReached:
      // Still in a state that needs inspection
      return false;
  }
  return false;
}

void P098_data_struct::findHome()
{
  pos_dest            = INT_MIN;
  limitA.switchposSet = false;
  startMoving();
}

void P098_data_struct::moveForward(int steps)
{
  if (steps <= 0) {
    pos_dest            = INT_MAX;
    limitB.switchposSet = false;
  } else {
    pos_dest = position + steps;
  }
  startMoving();
}

void P098_data_struct::moveReverse(int steps)
{
  if (steps > 0) {
    pos_dest = position - steps;
    startMoving();
  }
}

bool P098_data_struct::moveToPos(int pos)
{
  if (!canRun()) { return false; }
  const int offset = pos - getPosition();

  pos_dest = position + offset;
  startMoving();
  return true;
}

void P098_data_struct::stop()
{
  setPinState(_config.motorFwd, 0);
  setPinState(_config.motorRev, 0);
}

int P098_data_struct::getPosition() const
{
  if (limitA.switchposSet) {
    return position - limitA.switchpos;
  }
  return position;
}

void P098_data_struct::getLimitSwitchStates(bool& limitA_triggered, bool& limitB_triggered) const
{
  limitA_triggered = limitA.state == P098_limit_switch_state::State::High ? 1 : 0;
  limitB_triggered = limitB.state == P098_limit_switch_state::State::High ? 1 : 0;

  /*
     limitA_triggered = _config.limitA.readState();
     limitB_triggered = _config.limitB.readState();
   */
}

void P098_data_struct::getLimitSwitchPositions(int& limitApos, int& limitBpos) const
{
  limitApos = limitA.switchposSet ? limitA.switchpos : 0;
  limitBpos = limitB.switchposSet ? limitB.switchpos : 0;
}

void P098_data_struct::startMoving()
{
  // Stop first, to make sure both outputs will not be set high
  stop();

  if (pos_dest > position) {
    state = P098_data_struct::State::RunFwd;
    setPinState(_config.motorFwd, 1);
  } else {
    state = P098_data_struct::State::RunRev;
    setPinState(_config.motorRev, 1);
  }
}

void P098_data_struct::checkLimit(volatile P098_limit_switch_state& switch_state)
{
  if (switch_state.state == P098_limit_switch_state::State::High) {
    stop();
    state = P098_data_struct::State::StopLimitSw;
    return;
  }
  checkPosition();
}

void P098_data_struct::checkPosition()
{
  bool mustStop = false;

  switch (state) {
    case P098_data_struct::State::RunFwd:

      mustStop = (position >= pos_dest);
      break;
    case P098_data_struct::State::RunRev:

      mustStop = (position <= pos_dest);
      break;
    default:
      return;
  }

  if (mustStop) {
    stop();
    state = P098_data_struct::State::StopPosReached;

    /*
       // Correct for position error
       if (std::abs(position - pos_dest) > 10) {
       startMoving();
       }
     */
  }
}

void P098_data_struct::setPinState(const P098_GPIO_config& gpio_config, byte state)
{
  // FIXME TD-er: Must move this code to the ESPEasy core code.
  byte mode = PIN_MODE_OUTPUT;

  state = state == 0 ? gpio_config.low() : gpio_config.high();
  const uint32_t key = createKey(GPIO_PLUGIN_ID, gpio_config.gpio);

  if (globalMapPortStatus[key].mode != PIN_MODE_OFFLINE)
  {
    int8_t currentState;
    GPIO_Read(GPIO_PLUGIN_ID, gpio_config.gpio, currentState);

    if (currentState == -1) {
      mode  = PIN_MODE_OFFLINE;
      state = -1;
    }

    createAndSetPortStatus_Mode_State(key, mode, state);

    if (mode == PIN_MODE_OUTPUT)  {
      GPIO_Write(
        GPIO_PLUGIN_ID,
        gpio_config.gpio,
        state,
        mode);
    }
  }
}

bool P098_data_struct::setPinMode(const P098_GPIO_config& gpio_config)
{
  if (checkValidPortRange(GPIO_PLUGIN_ID, gpio_config.gpio)) {
    pinMode(gpio_config.gpio, gpio_config.pullUp ? INPUT_PULLUP : INPUT);
    return true;
  }
  return false;
}

void P098_data_struct::check_limit_switch(
  const P098_GPIO_config          & gpio_config,
  volatile P098_limit_switch_state& switch_state)
{
  // State is changed first in ISR, but compared after values are copied.
  const int triggerpos          = switch_state.triggerpos;
  const uint64_t lastChanged_us = switch_state.lastChanged_us;

  if (switch_state.state == P098_limit_switch_state::State::TriggerWaitBounce) {
    if (lastChanged_us != 0) {
      const uint64_t timeSinceLastTrigger = getMicros64() - lastChanged_us;

      if (timeSinceLastTrigger > gpio_config.debounceTime_us) {
        if (!switch_state.switchposSet) {
          switch_state.switchpos    = triggerpos;
          switch_state.switchposSet = true;
        }

        // Perform an extra check here on the state as it may have changed in the ISR call
        if (switch_state.state == P098_limit_switch_state::State::TriggerWaitBounce) {
          switch_state.state = P098_limit_switch_state::State::High;
        }
      }
    }
  }
}

void ICACHE_RAM_ATTR P098_data_struct::process_limit_switch(
  const P098_GPIO_config          & gpio_config,
  volatile P098_limit_switch_state& switch_state,
  int                               position)
{
  noInterrupts();
  {
    // Don't call gpio_config.readState() here
    const bool pinState        = gpio_config.inverted ? digitalRead(gpio_config.gpio) == 0 : digitalRead(gpio_config.gpio) != 0;
    const uint64_t currentTime = getMicros64();


    switch (switch_state.state) {
      case P098_limit_switch_state::State::Low:

        if (pinState) {
          switch_state.state          = P098_limit_switch_state::State::TriggerWaitBounce;
          switch_state.lastChanged_us = currentTime;
          switch_state.triggerpos     = position;
        }
        break;
      case P098_limit_switch_state::State::TriggerWaitBounce:
      {
        // Do not evaluate the debounce time here, evaluate in the loop
        if (pinState) {
          // Only situation we can get here is when we missed a low state interrupt.
          switch_state.lastChanged_us = currentTime;
          switch_state.triggerpos     = position;
        } else {
          switch_state.state          = P098_limit_switch_state::State::Low;
          switch_state.lastChanged_us = 0;
          switch_state.triggerpos     = 0;
        }
        break;
      }
      case P098_limit_switch_state::State::High:

        if (!pinState) {
          switch_state.state          = P098_limit_switch_state::State::Low;
          switch_state.lastChanged_us = 0;
          switch_state.triggerpos     = 0;
        }
        break;
    }
  }
  interrupts(); // enable interrupts again.
}

void ICACHE_RAM_ATTR P098_data_struct::ISRlimitA(P098_data_struct *self)
{
  process_limit_switch(self->_config.limitA, self->limitA, self->position);
}

void ICACHE_RAM_ATTR P098_data_struct::ISRlimitB(P098_data_struct *self)
{
  process_limit_switch(self->_config.limitB, self->limitB, self->position);
}

void ICACHE_RAM_ATTR P098_data_struct::ISRencoder(P098_data_struct *self)
{
  noInterrupts();

  switch (self->state) {
    case P098_data_struct::State::RunFwd:
      ++(self->position);
      break;
    case P098_data_struct::State::RunRev:
      --(self->position);
      break;
    default:
      break;
  }
  interrupts(); // enable interrupts again.
}

#endif // ifdef USES_P098
