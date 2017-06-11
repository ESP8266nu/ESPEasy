//#######################################################################################################
//#################### Plugin 050 I2C TCS34725 RGB Color Sensor with IR filter and White LED ############
//#######################################################################################################
//
// RGB Color Sensor with IR filter and White LED
// like this one: https://www.adafruit.com/products/1334
// based on this library: https://github.com/adafruit/Adafruit_TCS34725
// this code is based on 20170331 date version of the above library
//
// because of not changeable sensor address, i implemented support for the TCA9548A i2c multiplexer
// thanks to adafruit for the great tutorial on in
// https://learn.adafruit.com/adafruit-tca9548a-1-to-8-i2c-multiplexer-breakout/wiring-and-test
//
#ifdef PLUGIN_BUILD_TESTING

#include "Adafruit_TCS34725.h"

#define PLUGIN_050
#define PLUGIN_ID_050        50
#define PLUGIN_NAME_050       "Luminosity & Color - TCS34725  [TEST]"
#define PLUGIN_VALUENAME1_050 "Red"
#define PLUGIN_VALUENAME2_050 "Green"
#define PLUGIN_VALUENAME3_050 "Blue"
#define PLUGIN_VALUENAME4_050 "Color Temperature"

#define TCA9548A_ADDR 0x70

volatile boolean interruptFired = false;
unsigned long Plugin_050_pulseCounter = 0;


/*********************************************************************/
void Plugin_050_interrupt()
/*********************************************************************/
{
	interruptFired = true;
	Plugin_050_pulseCounter++;
}


boolean Plugin_050_init[TASKS_MAX];
int waitTime;
Adafruit_TCS34725 tcs;

boolean Plugin_050(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_050;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_QUAD;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 4;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_050);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_050));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_050));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_050));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_050));
        break;
      }


    case PLUGIN_WEBFORM_LOAD:
      {
        byte choiceMode = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String optionsMode[6];
        optionsMode[0] = F("TCS34725_INTEGRATIONTIME_2_4MS");
        optionsMode[1] = F("TCS34725_INTEGRATIONTIME_24MS");
        optionsMode[2] = F("TCS34725_INTEGRATIONTIME_50MS");
        optionsMode[3] = F("TCS34725_INTEGRATIONTIME_101MS");
        optionsMode[4] = F("TCS34725_INTEGRATIONTIME_154MS");
        optionsMode[5] = F("TCS34725_INTEGRATIONTIME_700MS");
        int optionValuesMode[6];
        optionValuesMode[0] = TCS34725_INTEGRATIONTIME_2_4MS;
        optionValuesMode[1] = TCS34725_INTEGRATIONTIME_24MS;
        optionValuesMode[2] = TCS34725_INTEGRATIONTIME_50MS;
        optionValuesMode[3] = TCS34725_INTEGRATIONTIME_101MS;
        optionValuesMode[4] = TCS34725_INTEGRATIONTIME_154MS;
        optionValuesMode[5] = TCS34725_INTEGRATIONTIME_700MS;
        addFormSelector(string, F("Integration Time"), F("plugin_050_integrationTime"), 6, optionsMode, optionValuesMode, choiceMode);

        byte choiceMode2 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String optionsMode2[4];
        optionsMode2[0] = F("TCS34725_GAIN_1X");
        optionsMode2[1] = F("TCS34725_GAIN_4X");
        optionsMode2[2] = F("TCS34725_GAIN_16X");
        optionsMode2[3] = F("TCS34725_GAIN_60X");
        int optionValuesMode2[4];
        optionValuesMode2[0] = TCS34725_GAIN_1X;
        optionValuesMode2[1] = TCS34725_GAIN_4X;
        optionValuesMode2[2] = TCS34725_GAIN_16X;
        optionValuesMode2[3] = TCS34725_GAIN_60X;
        addFormSelector(string, F("Gain"), F("plugin_050_gain"), 4, optionsMode2, optionValuesMode2, choiceMode2);

        string += F("<TR><TD>Enable LED:<TD>");
        addCheckBox(string, F("plugin_050_led_on"), Settings.TaskDevicePluginConfig[event->TaskIndex][2]);

        string += F("<TR><TD><hr><TD><hr>");

        string += F("<TR><TD>Interrupt Pin:<TD>");
        addPinSelect(false, string, "taskdevicepin3", Settings.TaskDevicePin3[event->TaskIndex]);
        string += F("<TR><TD>Low Threshold:<TD>");
        addNumericBox(string, F("plugin_050_intLowTreshold"), Settings.TaskDevicePluginConfig[event->TaskIndex][4]);
        string += F("<TR><TD>High Threshold:<TD>");
        addNumericBox(string, F("plugin_050_intHighTreshold"), Settings.TaskDevicePluginConfig[event->TaskIndex][5]);

        string += F("<TR><TD><hr><TD><hr>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg(F("plugin_050_integrationTime"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        String plugin2 = WebServer.arg(F("plugin_050_gain"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();

        String plugin3 = WebServer.arg(F("plugin_050_led_on"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = (plugin3 == F("on"));

        String plugin5 = WebServer.arg(F("plugin_050_intLowTreshold"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = plugin5.toInt();
        String plugin6 = WebServer.arg(F("plugin_050_intHighTreshold"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = plugin6.toInt();

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        pinMode(Settings.TaskDevicePin3[event->TaskIndex], INPUT_PULLUP); //TCS interrupt output is Active-LOW and Open-Drain
        attachInterrupt(Settings.TaskDevicePin3[event->TaskIndex], Plugin_050_interrupt, FALLING);

        //set multiplexer to correct port if
        if (Settings.i2c_multiplex_port[event->TaskIndex] != -1)
        {
        	addLog(LOG_LEVEL_DEBUG, String(F("Multiplexer Port: ")) + String(Settings.i2c_multiplex_port[event->TaskIndex]));
        	i2cMultiplexerSelect(Settings.i2c_multiplex_port[event->TaskIndex]);
        }
        else
        {
        	addLog(LOG_LEVEL_DEBUG, F("Multiplexer Off"));
        	i2cMultiplexerOff();
        }

      	tcs34725IntegrationTime_t integrationTime;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_2_4MS)
        {
        	integrationTime = TCS34725_INTEGRATIONTIME_2_4MS;
        	waitTime = 3;
        }
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_24MS)
        {
        	integrationTime = TCS34725_INTEGRATIONTIME_24MS;
        	waitTime = 25;
        }
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_50MS)
        {
        	integrationTime = TCS34725_INTEGRATIONTIME_50MS;
        	waitTime = 51;
        }
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_101MS)
        {
        	integrationTime = TCS34725_INTEGRATIONTIME_101MS;
        	waitTime = 102;
        }
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_154MS)
        {
        	integrationTime = TCS34725_INTEGRATIONTIME_154MS;
        	waitTime = 155;
        }
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_700MS)
        {
        	integrationTime = TCS34725_INTEGRATIONTIME_700MS;
        	waitTime = 701;
        }

        tcs34725Gain_t gain;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==TCS34725_GAIN_1X)
        	gain = TCS34725_GAIN_1X;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==TCS34725_GAIN_4X)
        	gain = TCS34725_GAIN_4X;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==TCS34725_GAIN_16X)
        	gain = TCS34725_GAIN_16X;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==TCS34725_GAIN_60X)
        	gain = TCS34725_GAIN_60X;

      	/* Initialise with specific int time and gain values */
      	tcs = Adafruit_TCS34725(integrationTime, gain);
        if (tcs.begin()) {
          Plugin_050_init[event->TaskIndex] = true;
          success = true;
        	addLog(LOG_LEVEL_DEBUG, String(F("TCS34725 init with Integr.Time: ")) + String(waitTime-1) +
        			F(" and Gain: ") + String(gain) + F(" completed"));

          tcs.write8(TCS34725_PERS, TCS34725_PERS_1_CYCLE);
          tcs.setIntLimits(Settings.TaskDevicePluginConfig[event->TaskIndex][4],
          		Settings.TaskDevicePluginConfig[event->TaskIndex][5]);
        	addLog(LOG_LEVEL_DEBUG, "TCS34725 Interrupt Pin:" +
        			String(Settings.TaskDevicePin3[event->TaskIndex]) +	" Tresholds LOW: " +
        			String(Settings.TaskDevicePluginConfig[event->TaskIndex][4]) +
        			F(" HIGH: ") + String(Settings.TaskDevicePluginConfig[event->TaskIndex][5]));

          tcs.setInterrupt(true);  // turn on LED
        }
        else {
					addLog(LOG_LEVEL_DEBUG, F("TCS34725 init failed, no sensor?"));
					Plugin_050_init[event->TaskIndex] = false;
	        success = false;
        }

        break;
      }

    case PLUGIN_TEN_PER_SECOND:
    {
    	tcs.clearInterrupt();
    	break;
    }

    case PLUGIN_READ:
      {
      	if (Plugin_050_init[event->TaskIndex])
      	{
      		//set multiplexer to correct port if
          if (Settings.i2c_multiplex_port[event->TaskIndex] != -1)
          {
          	addLog(LOG_LEVEL_DEBUG, String(F("Multiplexer Port: ")) + String(Settings.i2c_multiplex_port[event->TaskIndex]));
          	i2cMultiplexerSelect(Settings.i2c_multiplex_port[event->TaskIndex]);
          }
          else
          {
          	addLog(LOG_LEVEL_DEBUG, F("Multiplexer Off"));
          	i2cMultiplexerOff();
          }

					uint16_t r, g, b, c, colorTemp, lux;

//					if (Settings.TaskDevicePluginConfig[event->TaskIndex][2])
//						tcs.setInterrupt(false);      // turn on LED
//					else
//						tcs.setInterrupt(true);  // turn off LED
					delayBackground(waitTime);  // takes xx ms to read
					tcs.getRawData(&r, &g, &b, &c);
//					if (Settings.TaskDevicePluginConfig[event->TaskIndex][2])
//						tcs.setInterrupt(true);  // turn off LED
					colorTemp = tcs.calculateColorTemperature(r, g, b);
					lux = tcs.calculateLux(r, g, b);

					UserVar[event->BaseVarIndex] = r;
					UserVar[event->BaseVarIndex + 1] = g;
					UserVar[event->BaseVarIndex + 2] = b;
					UserVar[event->BaseVarIndex + 3] = colorTemp;

					String log = F("TCS34725: Color Temp (K): ");
					log += String(UserVar[event->BaseVarIndex + 3], DEC);
					log += F(" R: ");
					log += String(UserVar[event->BaseVarIndex], DEC);
					log += F(" G: ");
					log += String(UserVar[event->BaseVarIndex + 1], DEC);
					log += F(" B: ");
					log += String(UserVar[event->BaseVarIndex + 2], DEC);
					log += F(" C: ");
					log += String(c, DEC);
					addLog(LOG_LEVEL_INFO, log);
					addLog(LOG_LEVEL_INFO, "pulseCounter: " + String(Plugin_050_pulseCounter,DEC));
//					tcs.clearInterrupt();
					success = true;
      	}
        break;
      }

  }
  return success;
}


#endif
