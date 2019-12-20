#ifdef USES_C015
//#######################################################################################################
//########################### Controller Plugin 015: Blynk  #############################################
//#######################################################################################################

// This plugin provides blynk native protocol. This makes possible receive callbacks from user
// like button press, slider move etc.
// This require much more ESP resources, than use of blynk http API.
// So, use C012 Blynk HTTP plugin when you don't need blynk calbacks.
//
// Only one blynk controller instance is supported.
//
// https://www.youtube.com/watch?v=5_V_DibOypE

// #ifdef PLUGIN_BUILD_TESTING

// Uncomment this to use ssl connection. This requires more device resources than unencrypted one.
// Also it requires valid server thumbprint string to be entered in plugin settings.
// #define CPLUGIN_015_SSL

#define CPLUGIN_015
#define CPLUGIN_ID_015         15
#define _BLYNK_USE_DEFAULT_FREE_RAM
#define BLYNK_TIMEOUT_MS 2000UL
#define BLYNK_HEARTBEAT      30
#define CPLUGIN_015_RECONNECT_INTERVAL 60000

#ifdef CPLUGIN_015_SSL
  #include <BlynkSimpleEsp8266_SSL.h>
  #define CPLUGIN_NAME_015       "Blynk SSL [TESTING]"
  // Current official blynk server thumbprint
  #define CPLUGIN_015_DEFAULT_THUMBPRINT "FD C0 7D 8D 47 97 F7 E3 07 05 D3 4E E3 BB 8E 3D C0 EA BE 1C"
  #define C015_LOG_PREFIX "BL (ssl): "
#else
 #include <BlynkSimpleEsp8266.h>
 #define CPLUGIN_NAME_015       "Blynk [TESTING]"
 #define C015_LOG_PREFIX "BL: "
#endif



static unsigned long _C015_LastConnectAttempt[CONTROLLER_MAX] = {0,0,0};

void CPlugin_015_handleInterrupt(void) {
  // This cplugin uses modified blynk library.
  // It includes support of calling this during time-wait operations
  // like blynk connection process to keep espeasy stability.
  backgroundtasks(void);
}


void Blynk_Run_c015(void){
    // user callbacks processing. Called from run10TimesPerSecond.
    if (Blynk.connected(void))
      Blynk.run(void);
}


bool CPlugin_015(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_015;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = false;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_015);
        break;
      }

    case CPLUGIN_INIT:
      {
       // when connected to another server and user has changed settings
       if (Blynk.connected(void)){
          addLog(LOG_LEVEL_INFO, F(C015_LOG_PREFIX "disconnect from server"));
          Blynk.disconnect(void);
       }
       break;
      }

    #ifdef CPLUGIN_015_SSL
      case CPLUGIN_WEBFORM_LOAD:
        {
          char thumbprint[60];
          LoadCustomControllerSettings(event->ControllerIndex,(byte*)&thumbprint, sizeof(thumbprint));
          if (strlen(thumbprint) != 59)
              strcpy(thumbprint, CPLUGIN_015_DEFAULT_THUMBPRINT);
          addFormTextBox(F("Server thumbprint string"), F("c015_thumbprint"), thumbprint, 60);
          success = true;
          break;
        }
    #endif

    case CPLUGIN_WEBFORM_SAVE:
      {
        success = true;
        if (isFormItemChecked(F("controllerenabled"))){
          for (byte i = 0; i < CONTROLLER_MAX; ++i) {
            byte ProtocolIndex = getProtocolIndex(Settings.Protocol[i]);
            if (i != event->ControllerIndex && Protocol[ProtocolIndex].Number == 15 && Settings.ControllerEnabled[i]) {
              success = false;
              // FIXME:  this will only show a warning message and not uncheck "enabled" in webform.
              // Webserver object is not checking result of "success" var :(
              addHtmlError(F("Only one enabled instance of blynk controller is supported"));
              break;
            }
          }
          // force to connect without delay when webform saved
          _C015_LastConnectAttempt[event->ControllerIndex] = 0;

          #ifdef CPLUGIN_015_SSL
            char thumbprint[60];
            String error = F("Specify server thumbprint with exactly 59 symbols string like " CPLUGIN_015_DEFAULT_THUMBPRINT);
            if (!safe_strncpy(thumbprint, WebServer.arg("c015_thumbprint"), 60) || strlen(thumbprint) != 59) {
              addHtmlError(error);
            }
            SaveCustomControllerSettings(event->ControllerIndex,(byte*)&thumbprint, sizeof(thumbprint));
          #endif
        }
        break;
      }

     case CPLUGIN_PROTOCOL_SEND:
      {
        if (!Settings.ControllerEnabled[event->ControllerIndex])
          break;

        // Collect the values at the same run, to make sure all are from the same sample
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        C015_queue_element element(event, valueCount);
        if (ExtraTaskSettings.TaskIndex != event->TaskIndex) {
          String dummy;
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummy);
        }

        for (byte x = 0; x < valueCount; x++)
        {
          bool isvalid;
          String formattedValue = formatUserVar(event, x, isvalid);
          if (!isvalid)
            // send empty string to Blynk in case of error
            formattedValue = F("");

          String valueName = ExtraTaskSettings.TaskDeviceValueNames[x];
          String valueFullName = ExtraTaskSettings.TaskDeviceName;
          valueFullName += F(".");
          valueFullName += valueName;
          String vPinNumberStr = valueName.substring(1, 4);
          int vPinNumber = vPinNumberStr.toInt(void);
          String log = F(C015_LOG_PREFIX);
          log += Blynk.connected(void)? F("(online): ") : F("(offline): ");
          if (vPinNumber > 0 && vPinNumber < 256){
            log += F("send ");
            log += valueFullName;
            log += F(" = ");
            log += formattedValue;
            log += F(" to blynk pin v");
            log += vPinNumber;
          }
          else{
            vPinNumber = -1;
            log += F("error got vPin number for ");
            log += valueFullName;
            log += F(", got not valid value: ");
            log += vPinNumberStr;
          }
          addLog(LOG_LEVEL_INFO, log);
          element.vPin[x] = vPinNumber;
          element.txt[x] = formattedValue;
        }
        success = C015_DelayHandler.addToQueue(element);
        scheduleNextDelayQueue(TIMER_C015_DELAY_QUEUE, C015_DelayHandler.getNextScheduleTime(void));
        break;
      }
  }
  return success;
}

//********************************************************************************
// Process Queued Blynk request, with data set to NULL
//********************************************************************************
// controller_plugin_number = 015 because of C015
bool do_process_c015_delay_queue(int controller_plugin_number, const C015_queue_element& element, ControllerSettingsStruct& ControllerSettings);

bool do_process_c015_delay_queue(int controller_plugin_number, const C015_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  if (!Settings.ControllerEnabled[element.controller_idx])
    // controller has been disabled. Answer true to flush queue.
    return true;

  if (!WiFiConnected(void)) {
    return false;
  }

  if (!Blynk_keep_connection_c015(element.controller_idx, ControllerSettings))
    return false;

  while (element.vPin[element.valuesSent] == -1) {
  //   A non valid value, which we are not going to send.
  //   answer ok and skip real sending
    if (element.checkDone(true))
      return true;
  }

  return element.checkDone(Blynk_send_c015(element.txt[element.valuesSent], element.vPin[element.valuesSent]));
}


boolean Blynk_keep_connection_c015(int controllerIndex, ControllerSettingsStruct& ControllerSettings){
  if (!WiFiConnected(void))
    return false;

  if (!Blynk.connected(void)){
    String auth = SecuritySettings.ControllerPassword[controllerIndex];
    boolean connectDefault = false;

    if (timePassedSince(_C015_LastConnectAttempt[controllerIndex]) < CPLUGIN_015_RECONNECT_INTERVAL){
      //"skip connect to blynk server too often. Wait a little...";
      return false;
    }
    _C015_LastConnectAttempt[controllerIndex] = millis(void);

    #ifdef CPLUGIN_015_SSL
      char thumbprint[60];
      LoadCustomControllerSettings(controllerIndex,(byte*)&thumbprint, sizeof(thumbprint));
      if (strlen(thumbprint) != 59){
          addLog(LOG_LEVEL_INFO, C015_LOG_PREFIX "Saved thumprint value is not correct:");
          addLog(LOG_LEVEL_INFO, thumbprint);
          strcpy(thumbprint, CPLUGIN_015_DEFAULT_THUMBPRINT);
          addLog(LOG_LEVEL_INFO, C015_LOG_PREFIX "using default one:");
          addLog(LOG_LEVEL_INFO, thumbprint);
      }
    #endif

    String log = F(C015_LOG_PREFIX);

    if (ControllerSettings.UseDNS){
      String hostName = ControllerSettings.getHost(void);
      if (hostName.length(void) != 0){
        log += F("Connecting to custom blynk server ");
        log += ControllerSettings.getHostPortString(void);
        Blynk.config(auth.c_str(void),
                     CPlugin_015_handleInterrupt,
                     hostName.c_str(void),
                     ControllerSettings.Port
                     #ifdef CPLUGIN_015_SSL
                        ,thumbprint
                     #endif
                   );
      }
      else{
        log += F("Custom blynk server name not specified. ");
        connectDefault = true;
      }
    }
    else{
      IPAddress ip = ControllerSettings.getIP(void);
      if ((ip[0] + ip[1] + ip[2] + ip[3]) > 0){
        log += F("Connecting to custom blynk server ");
        log += ControllerSettings.getHostPortString(void);
        Blynk.config(auth.c_str(void),
                     CPlugin_015_handleInterrupt,
                     ip,
                     ControllerSettings.Port
                     #ifdef CPLUGIN_015_SSL
                        ,thumbprint
                     #endif
                   );
      }
      else{
        log += F("Custom blynk server ip not specified. ");
        connectDefault = true;
      }
    }
    addLog(LOG_LEVEL_INFO, log);

    if (connectDefault){
      addLog(LOG_LEVEL_INFO, F(C015_LOG_PREFIX "Connecting to default server"));
      Blynk.config(auth.c_str(void),
                   CPlugin_015_handleInterrupt,
                   BLYNK_DEFAULT_DOMAIN
                   #ifdef CPLUGIN_015_SSL
                      ,BLYNK_DEFAULT_PORT_SSL
                      ,thumbprint
                   #else
                      ,BLYNK_DEFAULT_PORT
                   #endif
                  );
    }

    #ifdef CPLUGIN_015_SSL
      if (!Blynk.connect(void)){
        if (!_blynkWifiClient.verify(thumbprint, BLYNK_DEFAULT_DOMAIN)){
          addLog(LOG_LEVEL_INFO, F(C015_LOG_PREFIX "thumbprint check FAILED! Check thumbprint in device settings and server thumbprint"));
          addLog(LOG_LEVEL_INFO, thumbprint);
        }
      }
    #else
      Blynk.connect(void);
    #endif
  }

  return Blynk.connected(void);
}


String Command_Blynk_Set_c015(struct EventStruct *event, const char* Line){

  // todo add multicontroller support and chek it is connected and enabled
  if (!Blynk.connected(void))
      return F("Not connected to blynk server");

  int vPin = event->Par1;

  if ((vPin < 0)  || (vPin > 255)){
    String err = F("Not correct blynk vPin number ");
    err += vPin;
    return err;
  }

  String data = parseString(Line, 3, true, false);

  if (data.length(void) == 0){
    String err = F("Skip sending empty data to blynk vPin ");
    err += vPin;
    return err;
  }

  String log = F(C015_LOG_PREFIX "(online): send blynk pin v");
  log += vPin;
  log += F(" = ");
  log += data;
  addLog(LOG_LEVEL_INFO, log);

  Blynk.virtualWrite(vPin, data);
  return return_command_success(void);
}


boolean Blynk_send_c015(const String& value, int vPin )
{
  Blynk.virtualWrite(vPin, value);
  unsigned long timer = millis(void) + Settings.MessageDelay;
  while (!timeOutReached(timer))
              backgroundtasks(void);
  return true;
}

// This is called for all virtual pins, that don't have BLYNK_WRITE handler
BLYNK_WRITE_DEFAULT(void) {
  byte vPin = request.pin;
  float pinValue = param.asFloat(void);
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F(C015_LOG_PREFIX "server set v");
    log += vPin;
    log += F(" to ");
    log += pinValue;
    addLog(LOG_LEVEL_INFO, log);
  }
  if (Settings.UseRules) {
    String eventCommand = F("blynkv");
    eventCommand += vPin;
    eventCommand += F("=");
    eventCommand += pinValue;
    eventQueue.add(eventCommand);
  }
}

BLYNK_CONNECTED(void) {
  // Your code here when hardware connects to Blynk Cloud or private server.
  // It’s common to call sync functions inside of this function.
  // Requests all stored on the server latest values for all widgets.
  if (Settings.UseRules) {
    String eventCommand = F("blynk_connected");
    eventQueue.add(eventCommand);
  }
  // addLog(LOG_LEVEL_INFO, F(C015_LOG_PREFIX "connected handler"));
}

// This is called when Smartphone App is opened
BLYNK_APP_CONNECTED(void) {
  if (Settings.UseRules) {
    String eventCommand = F("blynk_app_connected");
    eventQueue.add(eventCommand);
  }
  // addLog(LOG_LEVEL_INFO, F(C015_LOG_PREFIX "app connected handler"));
}

// This is called when Smartphone App is closed
BLYNK_APP_DISCONNECTED(void) {
  if (Settings.UseRules) {
    String eventCommand = F("blynk_app_disconnected");
    eventQueue.add(eventCommand);
  }
  // addLog(LOG_LEVEL_INFO, F(C015_LOG_PREFIX "app disconnected handler"));
}

#endif
