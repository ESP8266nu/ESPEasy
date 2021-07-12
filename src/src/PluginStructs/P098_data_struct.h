#ifndef PLUGINSTRUCTS_P098_DATA_STRUCT_H
#define PLUGINSTRUCTS_P098_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P098

# define P098_LIMIT_SWITCH_TRIGGERPOS_MARGIN 10

struct P098_GPIO_config {
  byte high() const {
    return inverted ? 0 : 1;
  }

  byte low() const {
    return inverted ? 1 : 0;
  }

  // Don't call this from ISR functions.
  bool readState() const {
    const bool state = digitalRead(gpio) != 0;

    return inverted ? !state : state;
  }

  uint64_t debounceTime_us = 100000;
  int      gpio            = -1;
  bool     pullUp          = false;
  bool     inverted        = false;
};
struct P098_config_struct {
  // Stored, so do not change values
  enum class PWM_mode_type {
    NoPWM = 0,
    PWM   = 1,

    MAX_TYPE
  };

  static const __FlashStringHelper* toString(P098_config_struct::PWM_mode_type PWM_mode);

  P098_GPIO_config motorFwd;
  P098_GPIO_config motorRev;

  P098_GPIO_config limitA;
  P098_GPIO_config limitB;
  P098_GPIO_config encoder;
  unsigned long    pwm_freq = 1000;

  int gpio_analogIn = -1;

  PWM_mode_type PWM_mode = PWM_mode_type::NoPWM;

  bool encoder_pu = false;
};

struct P098_limit_switch_state {
  // States:
  // - Low              : triggerpos 0, logic state = low
  // - TriggerWaitBounce: Stored triggerpos is preliminary, logic state = high
  // - TriggerRejected  : logic state = low within bounce timeout
  //                   -> clear triggerpos & clear timestamp
  //                   -> new state: Low
  // - High             : logic state was high for minimal debounce duration
  //                      triggerpos is accepted.
  //                      May also store the position of the limit switch.
  enum class State {
    Low,
    TriggerWaitBounce,
    High
  };

  uint64_t lastChanged_us = 0;
  int      triggerpos     = 0;
  int      switchpos      = 0;
  State    state          = State::Low;
  bool     switchposSet   = false;
};
struct P098_data_struct : public PluginTaskData_base {
  enum class State {
    Idle,
    RunFwd,
    RunRev,
    StopLimitSw,
    StopPosReached
  };

  P098_data_struct(const P098_config_struct& config);
  ~P098_data_struct();

  bool begin(int pos,
             int limitApos,
             int limitBpos);

  // Perform regular loop
  // Return false when state should be checked.
  bool loop();

  bool homePosSet() const;
  bool canRun();

  void findHome();

  // Run the motor N steps forward
  // N <= 0: Move till limit B switch is reached
  void moveForward(int steps);

  // Run the motor N steps in revere
  void moveReverse(int steps);

  // Move to absolute position.
  // May return false if the state was not (yet) cleared or position of limit switch is unknown.
  bool moveToPos(int pos);

  // Stop the motors and set the state to idle.
  void stop();

  int  getPosition() const;

  void getLimitSwitchStates(bool& limitA,
                            bool& limitB) const;

  void getLimitSwitchPositions(int& limitA,
                               int& limitB) const;


  State state       = State::Idle;
  bool  initialized = false;

private:

  const P098_config_struct         _config;
  volatile P098_limit_switch_state limitA;
  volatile P098_limit_switch_state limitB;
  volatile int                     position = 0;
  int                              pos_dest = 0;

  void        startMoving();

  void        checkLimit(volatile P098_limit_switch_state& switch_state);
  void        checkPosition();

  static void setPinState(const P098_GPIO_config& gpio_config,
                          byte                    state);

  static bool setPinMode(const P098_GPIO_config& gpio_config);

  static void check_limit_switch(
    const P098_GPIO_config          & gpio_config,
    volatile P098_limit_switch_state& switch_state);

  static void process_limit_switch(
    const P098_GPIO_config          & gpio_config,
    volatile P098_limit_switch_state& switch_state,
    int                               position);

  static void ISRlimitA(P098_data_struct *self);
  static void ISRlimitB(P098_data_struct *self);
  static void ISRencoder(P098_data_struct *self);
};

#endif // ifdef USES_P098
#endif // ifndef PLUGINSTRUCTS_P098_DATA_STRUCT_H
