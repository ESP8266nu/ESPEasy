//#######################################################################################################
//#################################### Plugin 083: MQ5 sensors ##########################################
//#######################################################################################################
#ifdef USES_P083
#define PLUGIN_083
#define PLUGIN_ID_083         83
#define PLUGIN_NAME_083       "Analog input - MQ5 gas sensor  [TESTING]"
#define PLUGIN_VALUENAME1_083 "Hydrogen"
#define PLUGIN_VALUENAME2_083 "LPG"
#define PLUGIN_VALUENAME3_083 "Methane"
#define PLUGIN_VALUENAME4_083 "CO"
#define PLUGIN_VALUENAME5_083 "Alcohol"

/************************Hardware Related Macros************************************/
#define RL_VALUE                     (1)     //define the load resistance on the board, in kilo ohms
#define RO_CLEAN_AIR_FACTOR          (6.455) //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet

/***********************Software Related Macros************************************/
// Number of samples and interval during calibration phase
#define CALIBARAION_SAMPLE_TIMES     (50)
#define CALIBRATION_SAMPLE_INTERVAL  (500)
// Number of samples and interval during normal operation
#define READ_SAMPLE_TIMES            (5)
#define READ_SAMPLE_INTERVAL         (50)

/**********************Application Related Macros**********************************/
#define GAS_HYDROGEN                 (0)
#define GAS_LPG                      (1)
#define GAS_METHANE                  (2)
#define GAS_CARBON_MONOXIDE          (3)
#define GAS_ALCOHOL                  (4)
// #define accuracy                     (0)    //for linearcurves
#define accuracy                   (1)    //for nonlinearcurves, un comment this line and comment the above line if calculations
                                                    //are to be done using non linear curve equations

#define P083_STAGE_NOT_MEASURING       0
#define P083_STAGE_TAKING_SAMPLES      1
#define P083_STAGE_TAKING_CALIBRATION  2
#define P083_STAGE_SAMPLE_READY        3

/*
Calibration  Remarks:
  This function assumes that the sensor is in clean air. It use
  MQResistanceCalculation to calculates the sensor resistance in clean air
  and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about
  10, which differs slightly between different sensors.
*/


struct P083_data_struct {
  P083_data_struct() :
      Ro(10.0), // Initial estimate before calibration is 10 kOhm
      Rsum(0.0),
      stage(P083_STAGE_NOT_MEASURING),
      samplenr(0),
      timer(0),
      samplesNeeded(READ_SAMPLE_TIMES),
      sampleInterval(READ_SAMPLE_INTERVAL),
      mq_pin(-1) {}

  void startMeasurement(int measurementType, int pinNum) {
    switch (measurementType) {
      case P083_STAGE_TAKING_SAMPLES:
        sampleInterval = READ_SAMPLE_INTERVAL;
        samplesNeeded = READ_SAMPLE_TIMES;
        break;
      case P083_STAGE_TAKING_CALIBRATION:
        sampleInterval = CALIBRATION_SAMPLE_INTERVAL;
        samplesNeeded = CALIBARAION_SAMPLE_TIMES;
        break;
    }
    mq_pin = pinNum;
    Rsum = 0.0;
    stage = measurementType;
    samplenr = 0;
    takeMeasurement();
  }

  // Take measurement and returns whether the measurement period is finished.
  bool takeMeasurement() {
    if (stage == P083_STAGE_TAKING_SAMPLES || stage == P083_STAGE_TAKING_CALIBRATION) {
      if (mq_pin >= 0) {
        Rsum += MQResistanceCalculation(analogRead(mq_pin));
        ++samplenr;
        timer = millis() + sampleInterval;
        if (samplenr >= samplesNeeded) {
          stage = P083_STAGE_SAMPLE_READY;
        }
      }
    }
    return stage == P083_STAGE_SAMPLE_READY;
  }

  float get() {
    if (samplenr == 0) return -1.0;
    return Rsum / samplenr;
  }

  bool calibrationActive() {
    return sampleInterval == CALIBRATION_SAMPLE_INTERVAL &&
           samplesNeeded == CALIBARAION_SAMPLE_TIMES;
  }

  /****************** MQResistanceCalculation ****************************************
  Input:   raw_adc - raw value read from adc, which represents the voltage
  Output:  the calculated sensor resistance
  Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
           across the load resistor and its resistance, the resistance of the sensor
           could be derived.
  ************************************************************************************/
  static float MQResistanceCalculation(int raw_adc) {
    return (((float)RL_VALUE * (1023 - raw_adc) / raw_adc));
  }

  float Ro;    // Value of calibrated resistor in kOhm
  float Rsum;    // Resistor value during measurements
  int stage;
  unsigned int samplenr;
  unsigned long timer;
  unsigned int samplesNeeded;
  unsigned long sampleInterval;
  int mq_pin;
};

P083_data_struct P083_data;

boolean Plugin_083(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_083;
        Device[deviceCount].Type = DEVICE_TYPE_ANALOG;
        Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 5;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_083);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_083));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_083));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_083));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_083));
        // Only 4 variables possible
//        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_083));
        break;
      }

    case PLUGIN_INIT:
      {
        int pinNum = Settings.TaskDevicePin1[event->TaskIndex];
        addLog(LOG_LEVEL_INFO, F("Calibrating...\n"));
        P083_data.startMeasurement(P083_STAGE_TAKING_CALIBRATION, pinNum);
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (P083_data.stage == P083_STAGE_TAKING_SAMPLES || P083_data.stage == P083_STAGE_TAKING_CALIBRATION) {
          if (timeOutReached(P083_data.timer)) {
            if (P083_data.takeMeasurement()) {
              if (P083_data.calibrationActive()) {
                // calibration is ready, store the Ro value.
                float RS_AIR_val = P083_data.get();
                if (RS_AIR_val > 0.0) {
                  RS_AIR_val = RS_AIR_val / CALIBARAION_SAMPLE_TIMES; // calculate the average value
                  P083_data.Ro = RS_AIR_val / RO_CLEAN_AIR_FACTOR; // RS_AIR_val divided by
                                                         // RO_CLEAN_AIR_FACTOR yields the Ro
                                                         // according to the chart in the datasheet
                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    String log = F("Calibration is done...\n");
                    log = F("Ro=");
                    log += P083_data.Ro;
                    log += F(" kohm\n");
                    addLog(LOG_LEVEL_INFO, log);
                  }
                }
              } else {
                // Measurement is done, schedule a new PLUGIN_READ call
                schedule_task_device_timer(event->TaskIndex, millis() + 10);
              }
            }
          }
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        success = P083_handleMeasurementReady(event);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        #if defined(ESP32)
          addHtml(F("<TR><TD>Analog Pin:<TD>"));
          addPinSelect(false, F("taskdevicepin1"), Settings.TaskDevicePin1[event->TaskIndex]);
        #endif
        success = true;
        break;
      }
  }
  return success;
}

bool P083_handleMeasurementReady(struct EventStruct *event) {
  bool success = false;
  switch (P083_data.stage) {
    case P083_STAGE_SAMPLE_READY:
    {
      float mqReadValue = P083_data.get() / P083_data.Ro;
      UserVar[event->BaseVarIndex]   = (float) MQGetGasPercentage(mqReadValue, GAS_HYDROGEN);
      UserVar[event->BaseVarIndex+1] = (float) MQGetGasPercentage(mqReadValue, GAS_LPG);
      UserVar[event->BaseVarIndex+2] = (float) MQGetGasPercentage(mqReadValue, GAS_METHANE);
      UserVar[event->BaseVarIndex+3] = (float) MQGetGasPercentage(mqReadValue, GAS_CARBON_MONOXIDE);
      // Only 4 variables possible
//      UserVar[event->BaseVarIndex+3] = (float) MQGetGasPercentage(mqReadValue, GAS_ALCOHOL);
      P083_data.stage = P083_STAGE_NOT_MEASURING;
      success = true;
      break;
    }
    case P083_STAGE_NOT_MEASURING:
    {
      int MQ5PIN = Settings.TaskDevicePin1[event->TaskIndex];
      P083_data.startMeasurement(P083_STAGE_TAKING_SAMPLES, MQ5PIN);
      break;
    }
  }
  return success;
}

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function uses different equations representing curves of each gas to
         calculate the ppm (parts per million) of the target gas.
************************************************************************************/
int MQGetGasPercentage(float rs_ro_ratio, int gas_id) {
  if (accuracy == 0) {
    if (gas_id == GAS_HYDROGEN) {
      return (pow(10, ((-3.986 * (log10(rs_ro_ratio))) + 3.075)));
    } else if (gas_id == GAS_LPG) {
      return (pow(10, ((-2.513 * (log10(rs_ro_ratio))) + 1.878)));
    } else if (gas_id == GAS_METHANE) {
      return (pow(10, ((-2.554 * (log10(rs_ro_ratio))) + 2.265)));
    } else if (gas_id == GAS_CARBON_MONOXIDE) {
      return (pow(10, ((-6.900 * (log10(rs_ro_ratio))) + 6.241)));
    } else if (gas_id == GAS_ALCOHOL) {
      return (pow(10, ((-4.590 * (log10(rs_ro_ratio))) + 4.851)));
    }
  } else if (accuracy == 1) {
    if (gas_id == GAS_HYDROGEN) {
      return (pow(10, (-22.89 * pow((log10(rs_ro_ratio)), 3) +
                       8.873 * pow((log10(rs_ro_ratio)), 2) -
                       3.587 * (log10(rs_ro_ratio)) + 2.948)));
    } else if (gas_id == GAS_LPG) {
      return (pow(10, ((-2.513 * (log10(rs_ro_ratio))) + 1.878)));
    } else if (gas_id == GAS_METHANE) {
      return (pow(10, (-0.428 * pow((log10(rs_ro_ratio)), 2) -
                       2.867 * (log10(rs_ro_ratio)) + 2.224)));
    } else if (gas_id == GAS_CARBON_MONOXIDE) {
      return (pow(10, (1401 * pow((log10(rs_ro_ratio)), 4) -
                       2777 * pow((log10(rs_ro_ratio)), 3) +
                       2059 * pow((log10(rs_ro_ratio)), 2) -
                       682.5 * (log10(rs_ro_ratio)) + 88.81)));
    } else if (gas_id == GAS_ALCOHOL) {
      return (pow(10, (14.90 * pow((log10(rs_ro_ratio)), 3) -
                       19.26 * pow((log10(rs_ro_ratio)), 2) +
                       3.108 * (log10(rs_ro_ratio)) + 3.922)));
    }
  }
  return 0;
}
#endif // USES_P083
