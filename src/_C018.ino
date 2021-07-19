#include "src/Helpers/_CPlugin_Helper.h"

#ifdef USES_C018

// #######################################################################################################
// ########################### Controller Plugin 018: LoRa TTN - RN2483/RN2903 ###########################
// #######################################################################################################

# define CPLUGIN_018
# define CPLUGIN_ID_018         18
# define CPLUGIN_NAME_018       "LoRa TTN - RN2483/RN2903 [TESTING]"
# define C018_BAUDRATE_LABEL     "baudrate"


# include <rn2xx3.h>
# include <ESPeasySerial.h>

# include "src/ControllerQueue/C018_queue_element.h"
# include "src/DataTypes/ESPEasy_plugin_functions.h"
# include "src/Globals/CPlugins.h"
# include "src/Globals/Protocol.h"
# include "src/Helpers/_Plugin_Helper_serial.h"
# include "src/Helpers/StringGenerator_GPIO.h"
# include "src/WebServer/Markup.h"
# include "src/WebServer/Markup_Forms.h"
# include "src/WebServer/HTML_wrappers.h"


// Have this define after the includes, so we can set it in Custom.h
# ifndef C018_FORCE_SW_SERIAL
#  define C018_FORCE_SW_SERIAL false
# endif // ifndef C018_FORCE_SW_SERIAL

struct C018_data_struct {
  C018_data_struct() : C018_easySerial(nullptr), myLora(nullptr) {}

  ~C018_data_struct() {
    reset();
  }

  void reset() {
    if (myLora != nullptr) {
      delete myLora;
      myLora = nullptr;
    }

    if (C018_easySerial != nullptr) {
      delete C018_easySerial;
      C018_easySerial = nullptr;
    }
    cacheDevAddr     = "";
    cacheHWEUI       = "";
    cacheSysVer      = "";
    autobaud_success = false;
  }

  bool init(const uint8_t port, const int8_t serial_rx, const int8_t serial_tx, unsigned long baudrate,
            bool joinIsOTAA, taskIndex_t sampleSet_Initiator, int8_t reset_pin) {
    if ((serial_rx < 0) || (serial_tx < 0)) {
      // Both pins are needed, or else no serial possible
      return false;
    }
    sampleSetInitiator = sampleSet_Initiator;

    if (isInitialized()) {
      // Check to see if serial parameters have changed.
      bool notChanged = true;
      notChanged &= C018_easySerial->getRxPin() == serial_rx;
      notChanged &= C018_easySerial->getTxPin() == serial_tx;
      notChanged &= C018_easySerial->getBaudRate() == baudrate;
      notChanged &= myLora->useOTAA() == joinIsOTAA;

      if (notChanged) { return true; }
    }
    reset();
    _resetPin = reset_pin;
    _baudrate = baudrate;

    // FIXME TD-er: Make force SW serial a proper setting.
    C018_easySerial = new (std::nothrow) ESPeasySerial(static_cast<ESPEasySerialPort>(port), serial_rx, serial_tx, false, 64);

    if (C018_easySerial != nullptr) {
      myLora = new rn2xx3(*C018_easySerial);
      myLora->setAsyncMode(true);
      myLora->setLastUsedJoinMode(joinIsOTAA);
      triggerAutobaud();
    }
    return isInitialized();
  }

  bool isInitialized() const {
    if ((C018_easySerial != nullptr) && (myLora != nullptr)) {
      if (autobaud_success) {
        return true;
      }
    }
    return false;
  }

  bool hasJoined() const {
    if (!isInitialized()) { return false; }
    return myLora->hasJoined();
  }

  bool useOTAA() const {
    if (!isInitialized()) { return true; }
    bool res = myLora->useOTAA();

    C018_logError(F("useOTA()"));
    return res;
  }

  bool command_finished() const {
    return myLora->command_finished();
  }

  bool txUncnfBytes(const uint8_t *data, uint8_t size, uint8_t port) {
    bool res = myLora->txBytes(data, size, port) != RN2xx3_datatypes::TX_return_type::TX_FAIL;

    C018_logError(F("txUncnfBytes()"));
    return res;
  }

  bool txHexBytes(const String& data, uint8_t port) {
    bool res = myLora->txHexBytes(data, port) != RN2xx3_datatypes::TX_return_type::TX_FAIL;

    C018_logError(F("txHexBytes()"));
    return res;
  }

  bool txUncnf(const String& data, uint8_t port) {
    bool res = myLora->tx(data, port) != RN2xx3_datatypes::TX_return_type::TX_FAIL;

    C018_logError(F("txUncnf()"));
    return res;
  }

  bool setTTNstack(RN2xx3_datatypes::TTN_stack_version version) {
    if (!isInitialized()) { return false; }
    bool res = myLora->setTTNstack(version);

    C018_logError(F("setTTNstack"));
    return res;
  }

  bool setFrequencyPlan(RN2xx3_datatypes::Freq_plan plan) {
    if (!isInitialized()) { return false; }
    bool res = myLora->setFrequencyPlan(plan);

    C018_logError(F("setFrequencyPlan()"));
    return res;
  }

  bool setSF(uint8_t sf) {
    if (!isInitialized()) { return false; }
    bool res = myLora->setSF(sf);

    C018_logError(F("setSF()"));
    return res;
  }

  bool setAdaptiveDataRate(bool enabled) {
    if (!isInitialized()) { return false; }
    bool res = myLora->setAdaptiveDataRate(enabled);

    C018_logError(F("setAdaptiveDataRate()"));
    return res;
  }


  bool initOTAA(const String& AppEUI = "", const String& AppKey = "", const String& DevEUI = "") {
    if (myLora == nullptr) { return false; }
    bool success = myLora->initOTAA(AppEUI, AppKey, DevEUI);

    C018_logError(F("initOTAA()"));
    updateCacheOnInit();
    return success;
  }

  bool initABP(const String& addr, const String& AppSKey, const String& NwkSKey) {
    if (myLora == nullptr) { return false; }
    bool success = myLora->initABP(addr, AppSKey, NwkSKey);

    C018_logError(F("initABP()"));
    updateCacheOnInit();
    return success;
  }

  String sendRawCommand(const String& command) {
    if (!isInitialized()) { return ""; }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("sendRawCommand: ");
      log += command;
      addLog(LOG_LEVEL_INFO, log);
    }
    String res = myLora->sendRawCommand(command);

    C018_logError(F("sendRawCommand()"));
    return res;
  }

  int getVbat() {
    if (!isInitialized()) { return -1; }
    return myLora->getVbat();
  }

  String peekLastError() {
    if (!isInitialized()) { return ""; }
    return myLora->peekLastError();
  }

  String getLastError() {
    if (!isInitialized()) { return ""; }
    return myLora->getLastError();
  }

  String getDataRate() {
    if (!isInitialized()) { return ""; }
    String res = myLora->getDataRate();

    C018_logError(F("getDataRate()"));
    return res;
  }

  int getRSSI() {
    if (!isInitialized()) { return 0; }
    return myLora->getRSSI();
  }

  uint32_t getRawStatus() {
    if (!isInitialized()) { return 0; }
    return myLora->getStatus().getRawStatus();
  }

  RN2xx3_status getStatus() const {
    if (!isInitialized()) { return RN2xx3_status(); }
    return myLora->getStatus();
  }

  bool getFrameCounters(uint32_t& dnctr, uint32_t& upctr) {
    if (!isInitialized()) { return false; }
    bool res = myLora->getFrameCounters(dnctr, upctr);

    C018_logError(F("getFrameCounters()"));
    return res;
  }

  bool setFrameCounters(uint32_t dnctr, uint32_t upctr) {
    if (!isInitialized()) { return false; }
    bool res = myLora->setFrameCounters(dnctr, upctr);

    C018_logError(F("setFrameCounters()"));
    return res;
  }

  // Cached data, only changing occasionally.

  String getDevaddr() {
    if (cacheDevAddr.isEmpty())
    {
      updateCacheOnInit();
    }
    return cacheDevAddr;
  }

  String hweui() {
    if (cacheHWEUI.isEmpty()) {
      if (isInitialized()) {
        cacheHWEUI = myLora->hweui();
      }
    }
    return cacheHWEUI;
  }

  String sysver() {
    if (cacheSysVer.isEmpty()) {
      if (isInitialized()) {
        cacheSysVer = myLora->sysver();
      }
    }
    return cacheSysVer;
  }

  uint8_t getSampleSetCount() const {
    return sampleSetCounter;
  }

  uint8_t getSampleSetCount(taskIndex_t taskIndex) {
    if (sampleSetInitiator == taskIndex)
    {
      ++sampleSetCounter;
    }
    return sampleSetCounter;
  }

  float getLoRaAirTime(uint8_t pl) const {
    if (isInitialized()) {
      return myLora->getLoRaAirTime(pl + 13); // We have a LoRaWAN header of 13 bytes.
    }
    return -1.0;
  }

  void async_loop() {
    if (isInitialized()) {
      rn2xx3_handler::RN_state state = myLora->async_loop();

      if (rn2xx3_handler::RN_state::must_perform_init == state) {
        if (myLora->get_busy_count() > 10) {
          if (_resetPin != -1) {
            pinMode(_resetPin, OUTPUT);
            digitalWrite(_resetPin, LOW);
            delay(50);
            digitalWrite(_resetPin, HIGH);
            delay(200);
          }
          autobaud_success = false;

          //          triggerAutobaud();
        }
      }
    }
  }

private:

  void C018_logError(const String& command) const {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String error = myLora->peekLastError();

      //    String error = myLora->getLastError();

      if (error.length() > 0) {
        String log = F("RN2483: ");
        log += command;
        log += ": ";
        log += error;
        addLog(LOG_LEVEL_INFO, log);
      }
    }
  }

  void updateCacheOnInit() {
    cacheDevAddr = "";

    if (isInitialized()) {
      if (myLora->getStatus().Joined)
      {
        cacheDevAddr = myLora->sendRawCommand(F("mac get devaddr"));

        if (cacheDevAddr == F("00000000")) {
          cacheDevAddr = "";
        }
      }
    }
  }

  void triggerAutobaud() {
    if ((C018_easySerial == nullptr) || (myLora == nullptr)) {
      return;
    }
    int retries = 2;

    while (retries > 0 && !autobaud_success) {
      if (retries == 1) {
        if (_resetPin != -1) {
          pinMode(_resetPin, OUTPUT);
          digitalWrite(_resetPin, LOW);
          delay(50);
          digitalWrite(_resetPin, HIGH);
          delay(200);
        }
      }

      // wakeUP_RN2483 and set data rate
      // Delay must be longer than specified in the datasheet for firmware 1.0.3
      // See: https://www.thethingsnetwork.org/forum/t/rn2483a-problems-no-serial-communication/7866/36?u=td-er

      // First set the baud rate low enough to even trigger autobaud when 9600 baud is active
      C018_easySerial->begin(600);
      C018_easySerial->write(static_cast<uint8_t>(0x00));

      // Set to desired baud rate.
      C018_easySerial->begin(_baudrate);
      C018_easySerial->write(static_cast<uint8_t>(0x55));
      C018_easySerial->println();
      delay(100);

      String response = myLora->sysver();

      // we could use sendRawCommand(F("sys get ver")); here
      //      C018_easySerial->println(F("sys get ver"));
      //      String response = C018_easySerial->readStringUntil('\n');
      autobaud_success = response.length() > 10;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("C018 AutoBaud: ");
        log += response;
        log += F(" status: ");
        log += myLora->sendRawCommand(F("mac get status"));
        addLog(LOG_LEVEL_INFO, log);
        C018_logError(F("autobaud check"));
      }
      --retries;
    }
  }

  ESPeasySerial *C018_easySerial = nullptr;
  rn2xx3        *myLora          = nullptr;
  String         cacheDevAddr;
  String         cacheHWEUI;
  String         cacheSysVer;
  unsigned long  _baudrate          = 57600;
  uint8_t        sampleSetCounter   = 0;
  taskIndex_t    sampleSetInitiator = INVALID_TASK_INDEX;
  int8_t         _resetPin          = -1;
  bool           autobaud_success   = false;
};

C018_data_struct *C018_data = nullptr;

# define C018_DEVICE_EUI_LEN          17
# define C018_DEVICE_ADDR_LEN         33
# define C018_NETWORK_SESSION_KEY_LEN 33
# define C018_APP_SESSION_KEY_LEN     33
# define C018_USE_OTAA                0
# define C018_USE_ABP                 1
struct C018_ConfigStruct
{
  C018_ConfigStruct() {
    reset();
  }

  void validate() {
    ZERO_TERMINATE(DeviceEUI);
    ZERO_TERMINATE(DeviceAddr);
    ZERO_TERMINATE(NetworkSessionKey);
    ZERO_TERMINATE(AppSessionKey);

    if ((baudrate < 2400) || (baudrate > 115200)) {
      reset();
    }
    if (stackVersion >= RN2xx3_datatypes::TTN_stack_version::TTN_NOT_SET) {
      stackVersion  = RN2xx3_datatypes::TTN_stack_version::TTN_v2;  
    }
  }

  void reset() {
    ZERO_FILL(DeviceEUI);
    ZERO_FILL(DeviceAddr);
    ZERO_FILL(NetworkSessionKey);
    ZERO_FILL(AppSessionKey);
    baudrate      = 9600;
    rxpin         = 12;
    txpin         = 14;
    resetpin      = -1;
    sf            = 7;
    frequencyplan = RN2xx3_datatypes::Freq_plan::TTN_EU;
    stackVersion  = RN2xx3_datatypes::TTN_stack_version::TTN_v2;
    joinmethod    = C018_USE_OTAA;
  }

  char          DeviceEUI[C018_DEVICE_EUI_LEN]                  = { 0 };
  char          DeviceAddr[C018_DEVICE_ADDR_LEN]                = { 0 };
  char          NetworkSessionKey[C018_NETWORK_SESSION_KEY_LEN] = { 0 };
  char          AppSessionKey[C018_APP_SESSION_KEY_LEN]         = { 0 };
  unsigned long baudrate                                        = 57600;
  int8_t        rxpin                                           = 12;
  int8_t        txpin                                           = 14;
  int8_t        resetpin                                        = -1;
  uint8_t       sf                                              = 7;
  uint8_t       frequencyplan                                   = RN2xx3_datatypes::Freq_plan::TTN_EU;
  uint8_t       joinmethod                                      = C018_USE_OTAA;
  uint8_t       serialPort                                      = 0;
  uint8_t       stackVersion                                    = RN2xx3_datatypes::TTN_stack_version::TTN_v2;
  uint8_t       adr                                             = 0;
};


bool CPlugin_018(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number       = CPLUGIN_ID_018;
      Protocol[protocolCount].usesMQTT       = false;
      Protocol[protocolCount].usesAccount    = true;
      Protocol[protocolCount].usesPassword   = true;
      Protocol[protocolCount].defaultPort    = 1;
      Protocol[protocolCount].usesID         = true;
      Protocol[protocolCount].usesHost       = false;
      Protocol[protocolCount].usesCheckReply = false;
      Protocol[protocolCount].usesTimeout    = false;
      Protocol[protocolCount].usesSampleSets = true;
      Protocol[protocolCount].needsNetwork   = false;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_018);
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
    {
      if ((C018_data != nullptr) && C018_data->isInitialized()) {
        string  = F("Dev addr: ");
        string += C018_data->getDevaddr();
        string += C018_data->useOTAA() ? F(" (OTAA)") : F(" (ABP)");
      } else {
        string = F("-");
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c018_delay_queue(event->ControllerIndex);

      if (success) {
        C018_init(event);
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      if (C018_data != nullptr) {
        C018_data->reset();
        delete C018_data;
        C018_data = nullptr;
      }
      exit_c018_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:
    {
      {
        // Script to toggle visibility of OTAA/ABP field, based on the activation method selector.
        protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(event->ControllerIndex);
        html_add_script(false);
        addHtml(F("function joinChanged(elem){ var styleOTAA = elem.value == 0 ? '' : 'none'; var styleABP = elem.value == 1 ? '' : 'none';"));
        addHtml(c018_add_joinChanged_script_element_line(getControllerParameterInternalName(ProtocolIndex, ControllerSettingsStruct::CONTROLLER_USER), true));
        addHtml(c018_add_joinChanged_script_element_line(getControllerParameterInternalName(ProtocolIndex, ControllerSettingsStruct::CONTROLLER_PASS), true));
        addHtml(c018_add_joinChanged_script_element_line(F("deveui"), true));
        addHtml(c018_add_joinChanged_script_element_line(F("deveui_note"), true));

        addHtml(c018_add_joinChanged_script_element_line(F("devaddr"), false));
        addHtml(c018_add_joinChanged_script_element_line(F("nskey"), false));
        addHtml(c018_add_joinChanged_script_element_line(F("appskey"), false));
        addHtml('}');
        html_add_script_end();
      }

      unsigned long baudrate;
      int8_t  rxpin;
      int8_t  txpin;
      int8_t  resetpin;
      uint8_t sf;
      uint8_t frequencyplan;
      uint8_t joinmethod;
      uint8_t stackVersion;
      uint8_t adr;

      {
        // Keep this object in a small scope so we can destruct it as soon as possible again.
        std::shared_ptr<C018_ConfigStruct> customConfig;
        {
          // Try to allocate on 2nd heap
          #ifdef USE_SECOND_HEAP
//          HeapSelectIram ephemeral;
          #endif
          std::shared_ptr<C018_ConfigStruct> tmp_shared(new (std::nothrow) C018_ConfigStruct);
          customConfig = std::move(tmp_shared);
        }

        if (!customConfig) {
          break;
        }
        LoadCustomControllerSettings(event->ControllerIndex, (uint8_t *)customConfig.get(), sizeof(C018_ConfigStruct));
        customConfig->validate();
        baudrate      = customConfig->baudrate;
        rxpin         = customConfig->rxpin;
        txpin         = customConfig->txpin;
        resetpin      = customConfig->resetpin;
        sf            = customConfig->sf;
        frequencyplan = customConfig->frequencyplan;
        joinmethod    = customConfig->joinmethod;
        stackVersion  = customConfig->stackVersion;
        adr           = customConfig->adr;

        {
          addFormTextBox(F("Device EUI"), F("deveui"), customConfig->DeviceEUI, C018_DEVICE_EUI_LEN - 1);
          String deveui_note = F("Leave empty to use HW DevEUI: ");

          if (C018_data != nullptr) {
            deveui_note += C018_data->hweui();
          }
          addFormNote(deveui_note, F("deveui_note"));
        }

        addFormTextBox(F("Device Addr"),         F("devaddr"), customConfig->DeviceAddr,        C018_DEVICE_ADDR_LEN - 1);
        addFormTextBox(F("Network Session Key"), F("nskey"),   customConfig->NetworkSessionKey, C018_NETWORK_SESSION_KEY_LEN - 1);
        addFormTextBox(F("App Session Key"),     F("appskey"), customConfig->AppSessionKey,     C018_APP_SESSION_KEY_LEN - 1);
      }

      {
        String options[2] = { F("OTAA"),  F("ABP") };
        int    values[2]  = { C018_USE_OTAA, C018_USE_ABP };
        addFormSelector_script(F("Activation Method"), F("joinmethod"), 2,
                               options, values, NULL, joinmethod,
                               F("joinChanged(this)")); // Script to toggle OTAA/ABP fields visibility when changing selection.
      }
      html_add_script(F("document.getElementById('joinmethod').onchange();"), false);

      addTableSeparator(F("Connection Configuration"), 2, 3);
      {
        const __FlashStringHelper * options[4] = { F("SINGLE_CHANNEL_EU"), F("TTN_EU"), F("TTN_US"), F("DEFAULT_EU") };
        int    values[4]  =
        {
          RN2xx3_datatypes::Freq_plan::SINGLE_CHANNEL_EU,
          RN2xx3_datatypes::Freq_plan::TTN_EU,
          RN2xx3_datatypes::Freq_plan::TTN_US,
          RN2xx3_datatypes::Freq_plan::DEFAULT_EU
        };

        addFormSelector(F("Frequency Plan"), F("frequencyplan"), 4, options, values, NULL, frequencyplan, false);
      }
      {
        const __FlashStringHelper * options[2] = { F("TTN v2"), F("TTN v3") };
        int    values[2]  = { 
          RN2xx3_datatypes::TTN_stack_version::TTN_v2,
          RN2xx3_datatypes::TTN_stack_version::TTN_v3
        };

        addFormSelector(F("TTN Stack"), F("ttnstack"), 2, options, values, NULL, stackVersion, false);
      }

      addFormNumericBox(F("Spread Factor"), F("sf"), sf, 7, 12);
      addFormCheckBox(F("Adaptive Data Rate (ADR)"), F("adr"), adr);


      addTableSeparator(F("Serial Port Configuration"), 2, 3);

      // Optional reset pin RN2xx3
      addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("Reset")), F("taskdevicepin3"), resetpin);

      // Show serial port selection
      addFormPinSelect(PinSelectPurpose::Generic_input, formatGpioName_RX(false),                   F("taskdevicepin1"), rxpin);
      addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_TX(false),                   F("taskdevicepin2"), txpin);

      // FIXME TD-er: Add port selector
      serialHelper_webformLoad(ESPEasySerialPort::not_set, rxpin, txpin, true);

      addFormNumericBox(F("Baudrate"), F(C018_BAUDRATE_LABEL), baudrate, 2400, 115200);
      addUnit(F("baud"));
      addFormNote(F("Module default baudrate: 57600 bps"));

      addTableSeparator(F("Device Status"), 2, 3);

      if (C018_data != nullptr) {
        // Some information on detected device
        addRowLabel(F("Hardware DevEUI"));
        addHtml(C018_data->hweui());
        addRowLabel(F("Version Number"));
        addHtml(C018_data->sysver());

        addRowLabel(F("Voltage"));
        addHtml(String(static_cast<float>(C018_data->getVbat()) / 1000.0, 3));

        addRowLabel(F("Dev Addr"));
        addHtml(C018_data->getDevaddr());

        uint32_t dnctr, upctr;

        if (C018_data->getFrameCounters(dnctr, upctr)) {
          addRowLabel(F("Frame Counters (down/up)"));
          String values = String(dnctr);
          values += '/';
          values += upctr;
          addHtml(values);
        }

        addRowLabel(F("Last Command Error"));
        addHtml(C018_data->getLastError());

        addRowLabel(F("Sample Set Counter"));
        addHtmlInt(C018_data->getSampleSetCount());

        addRowLabel(F("Data Rate"));
        addHtml(C018_data->getDataRate());        

        {
          RN2xx3_status status = C018_data->getStatus();

          addRowLabel(F("Status RAW value"));
          addHtml(String(status.getRawStatus()));

          addRowLabel(F("Activation Status"));
          addHtml(String(status.Joined));

          addRowLabel(F("Silent Immediately"));
          addHtml(String(status.SilentImmediately));
        }
      }


      break;
    }
    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:
    {
      std::shared_ptr<C018_ConfigStruct> customConfig;
      {
        // Try to allocate on 2nd heap
        #ifdef USE_SECOND_HEAP
//        HeapSelectIram ephemeral;
        #endif
        std::shared_ptr<C018_ConfigStruct> tmp_shared(new (std::nothrow) C018_ConfigStruct);
        customConfig = std::move(tmp_shared);
      }

      if (customConfig) {
        customConfig->reset();
        String deveui  = webArg(F("deveui"));
        String devaddr = webArg(F("devaddr"));
        String nskey   = webArg(F("nskey"));
        String appskey = webArg(F("appskey"));

        strlcpy(customConfig->DeviceEUI,         deveui.c_str(),  sizeof(customConfig->DeviceEUI));
        strlcpy(customConfig->DeviceAddr,        devaddr.c_str(), sizeof(customConfig->DeviceAddr));
        strlcpy(customConfig->NetworkSessionKey, nskey.c_str(),   sizeof(customConfig->NetworkSessionKey));
        strlcpy(customConfig->AppSessionKey,     appskey.c_str(), sizeof(customConfig->AppSessionKey));
        customConfig->baudrate      = getFormItemInt(F(C018_BAUDRATE_LABEL), customConfig->baudrate);
        customConfig->rxpin         = getFormItemInt(F("taskdevicepin1"), customConfig->rxpin);
        customConfig->txpin         = getFormItemInt(F("taskdevicepin2"), customConfig->txpin);
        customConfig->resetpin      = getFormItemInt(F("taskdevicepin3"), customConfig->resetpin);
        customConfig->sf            = getFormItemInt(F("sf"), customConfig->sf);
        customConfig->frequencyplan = getFormItemInt(F("frequencyplan"), customConfig->frequencyplan);
        customConfig->joinmethod    = getFormItemInt(F("joinmethod"), customConfig->joinmethod);
        customConfig->stackVersion  = getFormItemInt(F("ttnstack"), customConfig->stackVersion);
        customConfig->adr           = isFormItemChecked(F("adr"));
        serialHelper_webformSave(customConfig->serialPort, customConfig->rxpin, customConfig->txpin);
        SaveCustomControllerSettings(event->ControllerIndex, (uint8_t *)customConfig.get(), sizeof(C018_ConfigStruct));
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
    {
      success = true;

      switch (event->idx) {
        case ControllerSettingsStruct::CONTROLLER_USER:
          string = F("AppEUI");
          break;
        case ControllerSettingsStruct::CONTROLLER_PASS:
          string = F("AppKey");
          break;
        case ControllerSettingsStruct::CONTROLLER_TIMEOUT:
          string = F("Module Timeout");
          break;
        case ControllerSettingsStruct::CONTROLLER_PORT:
          string = F("Port");
          break;
        default:
          success = false;
          break;
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (C018_DelayHandler == nullptr) {
        break;
      }

      if (C018_data != nullptr) {
        success = C018_DelayHandler->addToQueue(
          std::move(C018_queue_element(event, C018_data->getSampleSetCount(event->TaskIndex))));
        Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C018_DELAY_QUEUE,
                                         C018_DelayHandler->getNextScheduleTime());

        if (!C018_data->isInitialized()) {
          // Sometimes the module does need some time after power on to respond.
          // So it may not be initialized well at the call of CPLUGIN_INIT
          // We try to trigger its init again when sending data.
          C018_init(event);
        }
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    {
      // FIXME TD-er: WHen should this be scheduled?
      // protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(event->ControllerIndex);
      // schedule_controller_event_timer(ProtocolIndex, CPlugin::Function::CPLUGIN_PROTOCOL_RECV, event);
      break;
    }

    case CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND:
    {
      if (C018_data != nullptr) {
        C018_data->async_loop();
      }

      // FIXME TD-er: Handle reading error state or return values.
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c018_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

bool C018_init(struct EventStruct *event) {
  String AppEUI;
  String AppKey;
  taskIndex_t  SampleSetInitiator = INVALID_TASK_INDEX;
  unsigned int Port               = 0;

  // Check if the object is already created.
  // If so, delete it to make sure the module is initialized according to the full set parameters.
  if (C018_data != nullptr) {
    C018_data->reset();
    delete C018_data;
    C018_data = nullptr;
  }


  if (C018_data == nullptr) {
    C018_data = new (std::nothrow) C018_data_struct;

    if (C018_data == nullptr) {
      return false;
    }
  }
  {
    // Allocate ControllerSettings object in a scope, so we can destruct it as soon as possible.
    MakeControllerSettings(ControllerSettings);

    if (!AllocatedControllerSettings()) {
      return false;
    }

    LoadControllerSettings(event->ControllerIndex, ControllerSettings);
    C018_DelayHandler->configureControllerSettings(ControllerSettings);
    AppEUI             = getControllerUser(event->ControllerIndex, ControllerSettings);
    AppKey             = getControllerPass(event->ControllerIndex, ControllerSettings);
    SampleSetInitiator = ControllerSettings.SampleSetInitiator;
    Port               = ControllerSettings.Port;
  }

  std::shared_ptr<C018_ConfigStruct> customConfig;
  {
    // Try to allocate on 2nd heap
    #ifdef USE_SECOND_HEAP
//    HeapSelectIram ephemeral;
    #endif
    std::shared_ptr<C018_ConfigStruct> tmp_shared(new (std::nothrow) C018_ConfigStruct);
    customConfig = std::move(tmp_shared);
  }

  if (!customConfig) {
    return false;
  }
  LoadCustomControllerSettings(event->ControllerIndex, (uint8_t *)customConfig.get(), sizeof(C018_ConfigStruct));
  customConfig->validate();

  if (!C018_data->init(customConfig->serialPort, customConfig->rxpin, customConfig->txpin, customConfig->baudrate,
                       (customConfig->joinmethod == C018_USE_OTAA),
                       SampleSetInitiator, customConfig->resetpin))
  {
    return false;
  }

  C018_data->setFrequencyPlan(static_cast<RN2xx3_datatypes::Freq_plan>(customConfig->frequencyplan));
  if (!C018_data->setSF(customConfig->sf)) {
    return false;
  }
  if (!C018_data->setAdaptiveDataRate(customConfig->adr != 0)) {
    return false;
  }

  if (customConfig->joinmethod == C018_USE_OTAA) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("OTAA: AppEUI: ");
      log += AppEUI;
      log += F(" AppKey: ");
      log += AppKey;
      log += F(" DevEUI: ");
      log += customConfig->DeviceEUI;

      addLog(LOG_LEVEL_INFO, log);
    }

    if (!C018_data->initOTAA(AppEUI, AppKey, customConfig->DeviceEUI)) {
      return false;
    }
  }
  else {
    if (!C018_data->initABP(customConfig->DeviceAddr, customConfig->AppSessionKey, customConfig->NetworkSessionKey)) {
      return false;
    }
  }

  if (!C018_data->setTTNstack(static_cast<RN2xx3_datatypes::TTN_stack_version>(customConfig->stackVersion))) {
    return false;
  }

  if (!C018_data->txUncnf(F("ESPeasy (TTN)"), Port)) {
    return false;
  }
  return true;
}

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c018_delay_queue(int controller_number, const C018_queue_element& element, ControllerSettingsStruct& ControllerSettings);

bool do_process_c018_delay_queue(int controller_number, const C018_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
// *INDENT-ON*
  uint8_t pl           = (element.packed.length() / 2);
  float   airtime_ms   = C018_data->getLoRaAirTime(pl);
  bool    mustSetDelay = false;
  bool    success      = false;

  if (!C018_data->command_finished()) {
    mustSetDelay = true;
  } else {
    success = C018_data->txHexBytes(element.packed, ControllerSettings.Port);

    if (success) {
      if (airtime_ms > 0.0) {
        ADD_TIMER_STAT(C018_AIR_TIME, static_cast<unsigned long>(airtime_ms * 1000));

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("LoRaWAN : Payload Length: ");
          log += pl + 13; // We have a LoRaWAN header of 13 bytes.
          log += F(" Air Time: ");
          log += String(airtime_ms, 3);
          log += F(" ms");
          addLog(LOG_LEVEL_INFO, log);
        }
      }
    }
  }
  String error = C018_data->getLastError(); // Clear the error string.

  if (error.indexOf(F("no_free_ch")) != -1) {
    mustSetDelay = true;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("C018 : Sent: ");
    log += element.packed;
    log += F(" length: ");
    log += String(element.packed.length());

    if (success) {
      log += F(" (success) ");
    }
    log += error;
    addLog(LOG_LEVEL_INFO, log);
  }

  if (mustSetDelay) {
    // Module is still sending, delay for 10x expected air time, which is equivalent of 10% air time duty cycle.
    // This can be retried a few times, so at most 10 retries like these are needed to get below 1% air time again.
    // Very likely only 2 - 3 of these delays are needed, as we have 8 channels to send from and messages are likely sent in bursts.
    C018_DelayHandler->setAdditionalDelay(10 * airtime_ms);

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("LoRaWAN : Unable to send. Delay for ");
      log += 10 * airtime_ms;
      log += F(" ms");
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  return success;
}

String c018_add_joinChanged_script_element_line(const String& id, bool forOTAA) {
  String result = F("document.getElementById('tr_");

  result += id;
  result += F("').style.display = style");
  result += forOTAA ? F("OTAA") : F("ABP");
  result += ';';
  return result;
}

#endif // ifdef USES_C018
