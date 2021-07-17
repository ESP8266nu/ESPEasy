/* Library for reading SDM 120/220/230/630 Modbus Energy meters.
*  Reading via Hardware or Software Serial library & rs232<->rs485 converter
*  2016-2018 Reaper7 (tested on wemos d1 mini->ESP8266 with Arduino 1.9.0-beta & 2.4.1 esp8266 core)
*  crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino/)
*/
//------------------------------------------------------------------------------
#ifndef SDM_h
#define SDM_h
//------------------------------------------------------------------------------
#include <Arduino.h>
#include <SDM_Config_User.h>
#ifdef USE_HARDWARESERIAL
  #include <HardwareSerial.h>
#else
  #include <ESPeasySerial.h>
//  #include <SoftwareSerial.h>
#endif
//------------------------------------------------------------------------------
//DEFAULT CONFIG (DO NOT CHANGE ANYTHING!!! for changes use SDM_Config_User.h):
//------------------------------------------------------------------------------
#ifndef SDM_UART_BAUD
  #define SDM_UART_BAUD                     4800                                //default baudrate
#endif

#ifndef DERE_PIN
  #define DERE_PIN                          NOT_A_PIN                           //default digital pin for control MAX485 DE/RE lines (connect DE & /RE together to this pin)
#endif

#ifdef USE_HARDWARESERIAL

  #ifndef SDM_UART_CONFIG
    #define SDM_UART_CONFIG                 SERIAL_8N1                          //default hardware uart config
  #endif

  #ifndef SWAPHWSERIAL
    #define SWAPHWSERIAL                    0                                   //(only esp8266) when hwserial used, then swap uart pins from 3/1 to 13/15 (default not swap)
  #endif

#endif

#ifndef MAX_MILLIS_TO_WAIT
  #define MAX_MILLIS_TO_WAIT                500                                 //default max time to wait for response from SDM. Try to get as low as possible with higest possible Baud Rate to decrease loop time. 
#endif
//------------------------------------------------------------------------------

#define SDM_ERR_NO_ERROR                              0                         //  no error
#define SDM_ERR_CRC_ERROR                             1                         //  crc error
#define SDM_ERR_WRONG_BYTES                           2                         //  bytes b0,b1 or b2 wrong
#define SDM_ERR_NOT_ENOUGHT_BYTES                     3                         //  not enough bytes from sdm
#define SDM_ERR_TIMEOUT                               4                         //  timeout

//------------------------------------------------------------------------------

#define FRAMESIZE                                     9                         //  size of out/in array
#define SDM_REPLY_BYTE_COUNT                          0x04                      //  number of bytes with data

#define SDM_B_01                                      0x01                      //  BYTE 1 -> slave address (default value 1 read from node 1)
#define SDM_B_02                                      0x04                      //  BYTE 2 -> function code (default value 0x04 read from 3X input registers)
#define SDM_B_05                                      0x00                      //  BYTE 5
#define SDM_B_06                                      0x02                      //  BYTE 6
                                                                                //  BYTES 3 & 4 (BELOW)

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
//      REGISTERS LIST FOR SDM DEVICES                                                                                                                      |
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
//      REGISTER NAME                                 REGISTER ADDRESS              UNIT        | SDM630  | SDM230  | SDM220  | SDM120CT| SDM120  | SDM72D  |
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
#define SDM_PHASE_1_VOLTAGE                           0x0000                    //  V           |    1    |    1    |    1    |    1    |    1    |         |
#define SDM_PHASE_2_VOLTAGE                           0x0002                    //  V           |    1    |         |         |         |         |         |
#define SDM_PHASE_3_VOLTAGE                           0x0004                    //  V           |    1    |         |         |         |         |         |
#define SDM_PHASE_1_CURRENT                           0x0006                    //  A           |    1    |    1    |    1    |    1    |    1    |         |
#define SDM_PHASE_2_CURRENT                           0x0008                    //  A           |    1    |         |         |         |         |         |
#define SDM_PHASE_3_CURRENT                           0x000A                    //  A           |    1    |         |         |         |         |         |
#define SDM_PHASE_1_POWER                             0x000C                    //  W           |    1    |    1    |    1    |    1    |    1    |         |
#define SDM_PHASE_2_POWER                             0x000E                    //  W           |    1    |         |         |         |         |         |
#define SDM_PHASE_3_POWER                             0x0010                    //  W           |    1    |         |         |         |         |         |
#define SDM_PHASE_1_APPARENT_POWER                    0x0012                    //  VA          |    1    |    1    |    1    |    1    |    1    |         |
#define SDM_PHASE_2_APPARENT_POWER                    0x0014                    //  VA          |    1    |         |         |         |         |         |
#define SDM_PHASE_3_APPARENT_POWER                    0x0016                    //  VA          |    1    |         |         |         |         |         |
#define SDM_PHASE_1_REACTIVE_POWER                    0x0018                    //  VAr         |    1    |    1    |    1    |    1    |    1    |         |
#define SDM_PHASE_2_REACTIVE_POWER                    0x001A                    //  VAr         |    1    |         |         |         |         |         |
#define SDM_PHASE_3_REACTIVE_POWER                    0x001C                    //  VAr         |    1    |         |         |         |         |         |
#define SDM_PHASE_1_POWER_FACTOR                      0x001E                    //              |    1    |    1    |    1    |    1    |    1    |         |
#define SDM_PHASE_2_POWER_FACTOR                      0x0020                    //              |    1    |         |         |         |         |         |
#define SDM_PHASE_3_POWER_FACTOR                      0x0022                    //              |    1    |         |         |         |         |         |
#define SDM_PHASE_1_ANGLE                             0x0024                    //  Degrees     |    1    |    1    |    1    |    1    |         |         |
#define SDM_PHASE_2_ANGLE                             0x0026                    //  Degrees     |    1    |         |         |         |         |         |
#define SDM_PHASE_3_ANGLE                             0x0028                    //  Degrees     |    1    |         |         |         |         |         |
#define SDM_AVERAGE_L_TO_N_VOLTS                      0x002A                    //  V           |    1    |         |         |         |         |         |
#define SDM_AVERAGE_LINE_CURRENT                      0x002E                    //  A           |    1    |         |         |         |         |         |
#define SDM_SUM_LINE_CURRENT                          0x0030                    //  A           |    1    |         |         |         |         |         |
#define SDM_TOTAL_SYSTEM_POWER                        0x0034                    //  W           |    1    |         |         |         |         |    1    |
#define SDM_TOTAL_SYSTEM_APPARENT_POWER               0x0038                    //  VA          |    1    |         |         |         |         |         |
#define SDM_TOTAL_SYSTEM_REACTIVE_POWER               0x003C                    //  VAr         |    1    |         |         |         |         |         |
#define SDM_TOTAL_SYSTEM_POWER_FACTOR                 0x003E                    //              |    1    |         |         |         |         |         |
#define SDM_TOTAL_SYSTEM_PHASE_ANGLE                  0x0042                    //  Degrees     |    1    |         |         |         |         |         |
#define SDM_FREQUENCY                                 0x0046                    //  Hz          |    1    |    1    |    1    |    1    |    1    |         |
#define SDM_IMPORT_ACTIVE_ENERGY                      0x0048                    //  kWh/MWh     |    1    |    1    |    1    |    1    |    1    |    1    |
#define SDM_EXPORT_ACTIVE_ENERGY                      0x004A                    //  kWh/MWh     |    1    |    1    |    1    |    1    |    1    |    1    |
#define SDM_IMPORT_REACTIVE_ENERGY                    0x004C                    //  kVArh/MVArh |    1    |    1    |    1    |    1    |    1    |         |
#define SDM_EXPORT_REACTIVE_ENERGY                    0x004E                    //  kVArh/MVArh |    1    |    1    |    1    |    1    |    1    |         |
#define SDM_VAH_SINCE_LAST_RESET                      0x0050                    //  kVAh/MVAh   |    1    |         |         |         |         |         |
#define SDM_AH_SINCE_LAST_RESET                       0x0052                    //  Ah/kAh      |    1    |         |         |         |         |         |
#define SDM_TOTAL_SYSTEM_POWER_DEMAND                 0x0054                    //  W           |    1    |    1    |         |         |         |         |
#define SDM_MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND         0x0056                    //  W           |    1    |    1    |         |         |         |         |
#define SDM_CURRENT_SYSTEM_POSITIVE_POWER_DEMAND      0x0058                    //  W           |         |    1    |         |         |         |         |
#define SDM_MAXIMUM_SYSTEM_POSITIVE_POWER_DEMAND      0x005A                    //  W           |         |    1    |         |         |         |         |
#define SDM_CURRENT_SYSTEM_REVERSE_POWER_DEMAND       0x005C                    //  W           |         |    1    |         |         |         |         |
#define SDM_MAXIMUM_SYSTEM_REVERSE_POWER_DEMAND       0x005E                    //  W           |         |    1    |         |         |         |         |
#define SDM_TOTAL_SYSTEM_VA_DEMAND                    0x0064                    //  VA          |    1    |         |         |         |         |         |
#define SDM_MAXIMUM_TOTAL_SYSTEM_VA_DEMAND            0x0066                    //  VA          |    1    |         |         |         |         |         |
#define SDM_NEUTRAL_CURRENT_DEMAND                    0x0068                    //  A           |    1    |         |         |         |         |         |
#define SDM_MAXIMUM_NEUTRAL_CURRENT                   0x006A                    //  A           |    1    |         |         |         |         |         |
#define SDM_LINE_1_TO_LINE_2_VOLTS                    0x00C8                    //  V           |    1    |         |         |         |         |         |
#define SDM_LINE_2_TO_LINE_3_VOLTS                    0x00CA                    //  V           |    1    |         |         |         |         |         |
#define SDM_LINE_3_TO_LINE_1_VOLTS                    0x00CC                    //  V           |    1    |         |         |         |         |         |
#define SDM_AVERAGE_LINE_TO_LINE_VOLTS                0x00CE                    //  V           |    1    |         |         |         |         |         |
#define SDM_NEUTRAL_CURRENT                           0x00E0                    //  A           |    1    |         |         |         |         |         |
#define SDM_PHASE_1_LN_VOLTS_THD                      0x00EA                    //  %           |    1    |         |         |         |         |         |
#define SDM_PHASE_2_LN_VOLTS_THD                      0x00EC                    //  %           |    1    |         |         |         |         |         |
#define SDM_PHASE_3_LN_VOLTS_THD                      0x00EE                    //  %           |    1    |         |         |         |         |         |
#define SDM_PHASE_1_CURRENT_THD                       0x00F0                    //  %           |    1    |         |         |         |         |         |
#define SDM_PHASE_2_CURRENT_THD                       0x00F2                    //  %           |    1    |         |         |         |         |         |
#define SDM_PHASE_3_CURRENT_THD                       0x00F4                    //  %           |    1    |         |         |         |         |         |
#define SDM_AVERAGE_LINE_TO_NEUTRAL_VOLTS_THD         0x00F8                    //  %           |    1    |         |         |         |         |         |
#define SDM_AVERAGE_LINE_CURRENT_THD                  0x00FA                    //  %           |    1    |         |         |         |         |         |
#define SDM_TOTAL_SYSTEM_POWER_FACTOR_INV             0x00FE                    //              |    1    |         |         |         |         |         |
#define SDM_PHASE_1_CURRENT_DEMAND                    0x0102                    //  A           |    1    |    1    |         |         |         |         |
#define SDM_PHASE_2_CURRENT_DEMAND                    0x0104                    //  A           |    1    |         |         |         |         |         |
#define SDM_PHASE_3_CURRENT_DEMAND                    0x0106                    //  A           |    1    |         |         |         |         |         |
#define SDM_MAXIMUM_PHASE_1_CURRENT_DEMAND            0x0108                    //  A           |    1    |    1    |         |         |         |         |
#define SDM_MAXIMUM_PHASE_2_CURRENT_DEMAND            0x010A                    //  A           |    1    |         |         |         |         |         |
#define SDM_MAXIMUM_PHASE_3_CURRENT_DEMAND            0x010C                    //  A           |    1    |         |         |         |         |         |
#define SDM_LINE_1_TO_LINE_2_VOLTS_THD                0x014E                    //  %           |    1    |         |         |         |         |         |
#define SDM_LINE_2_TO_LINE_3_VOLTS_THD                0x0150                    //  %           |    1    |         |         |         |         |         |
#define SDM_LINE_3_TO_LINE_1_VOLTS_THD                0x0152                    //  %           |    1    |         |         |         |         |         |
#define SDM_AVERAGE_LINE_TO_LINE_VOLTS_THD            0x0154                    //  %           |    1    |         |         |         |         |         |
#define SDM_TOTAL_ACTIVE_ENERGY                       0x0156                    //  kWh         |    1    |    1    |    1    |    1    |    1    |    1    |
#define SDM_TOTAL_REACTIVE_ENERGY                     0x0158                    //  kVArh       |    1    |    1    |    1    |    1    |    1    |         |
#define SDM_L1_IMPORT_ACTIVE_ENERGY                   0x015A                    //  kWh         |    1    |         |         |         |         |         |
#define SDM_L2_IMPORT_ACTIVE_ENERGY                   0x015C                    //  kWh         |    1    |         |         |         |         |         |
#define SDM_L3_IMPORT_ACTIVE_ENERGY                   0x015E                    //  kWh         |    1    |         |         |         |         |         |
#define SDM_L1_EXPORT_ACTIVE_ENERGY                   0x0160                    //  kWh         |    1    |         |         |         |         |         |
#define SDM_L2_EXPORT_ACTIVE_ENERGY                   0x0162                    //  kWh         |    1    |         |         |         |         |         |
#define SDM_L3_EXPORT_ACTIVE_ENERGY                   0x0164                    //  kWh         |    1    |         |         |         |         |         |
#define SDM_L1_TOTAL_ACTIVE_ENERGY                    0x0166                    //  kWh         |    1    |         |         |         |         |         |
#define SDM_L2_TOTAL_ACTIVE_ENERGY                    0x0168                    //  kWh         |    1    |         |         |         |         |         |
#define SDM_L3_TOTAL_ACTIVE_ENERGY                    0x016a                    //  kWh         |    1    |         |         |         |         |         |
#define SDM_L1_IMPORT_REACTIVE_ENERGY                 0x016C                    //  kVArh       |    1    |         |         |         |         |         |
#define SDM_L2_IMPORT_REACTIVE_ENERGY                 0x016E                    //  kVArh       |    1    |         |         |         |         |         |
#define SDM_L3_IMPORT_REACTIVE_ENERGY                 0x0170                    //  kVArh       |    1    |         |         |         |         |         |
#define SDM_L1_EXPORT_REACTIVE_ENERGY                 0x0172                    //  kVArh       |    1    |         |         |         |         |         |
#define SDM_L2_EXPORT_REACTIVE_ENERGY                 0x0174                    //  kVArh       |    1    |         |         |         |         |         |
#define SDM_L3_EXPORT_REACTIVE_ENERGY                 0x0176                    //  kVArh       |    1    |         |         |         |         |         |
#define SDM_L1_TOTAL_REACTIVE_ENERGY                  0x0178                    //  kVArh       |    1    |         |         |         |         |         |
#define SDM_L2_TOTAL_REACTIVE_ENERGY                  0x017A                    //  kVArh       |    1    |         |         |         |         |         |
#define SDM_L3_TOTAL_REACTIVE_ENERGY                  0x017C                    //  kVArh       |    1    |         |         |         |         |         |
#define SDM_CURRENT_RESETTABLE_TOTAL_ACTIVE_ENERGY    0x0180                    //  kWh         |         |    1    |         |         |         |    1    |
#define SDM_CURRENT_RESETTABLE_TOTAL_REACTIVE_ENERGY  0x0182                    //  kVArh       |         |    1    |         |         |         |         |
#define SDM_CURRENT_RESETTABLE_IMPORT_ENERGY          0x0184                    //  kWh         |         |         |         |         |         |    1    |
#define SDM_CURRENT_RESETTABLE_EXPORT_ENERGY          0x0186                    //  kWh         |         |         |         |         |         |    1    |
#define SDM_IMPORT_POWER                              0x0500                    //  W           |         |         |         |         |         |    1    |
#define SDM_EXPORT_POWER                              0x0502                    //  W           |         |         |         |         |         |    1    |
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//      REGISTERS LIST FOR DDM DEVICE                                                                     |
//---------------------------------------------------------------------------------------------------------
//      REGISTER NAME                                 REGISTER ADDRESS              UNIT        | DDM18SD |
//---------------------------------------------------------------------------------------------------------
#define DDM_PHASE_1_VOLTAGE                           0x0000                    //  V           |    1    |
#define DDM_PHASE_1_CURRENT                           0x0008                    //  A           |    1    |
#define DDM_PHASE_1_POWER                             0x0012                    //  W           |    1    |
#define DDM_PHASE_1_REACTIVE_POWER                    0x001A                    //  VAr         |    1    |
#define DDM_PHASE_1_POWER_FACTOR                      0x002A                    //              |    1    |
#define DDM_FREQUENCY                                 0x0036                    //  Hz          |    1    |
#define DDM_IMPORT_ACTIVE_ENERGY                      0x0100                    //  kWh         |    1    |
#define DDM_IMPORT_REACTIVE_ENERGY                    0x0400                    //  kVArh       |    1    |
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//      REGISTERS LIST FOR DEVNAME DEVICE                                                                 |
//---------------------------------------------------------------------------------------------------------
//      REGISTER NAME                                 REGISTER ADDRESS              UNIT        | DEVNAME |
//---------------------------------------------------------------------------------------------------------
//#define DEVNAME_VOLTAGE                             0x0000                    //  V           |    1    |
//#define DEVNAME_CURRENT                             0x0002                    //  A           |    1    |
//#define DEVNAME_POWER                               0x0004                    //  W           |    1    |
//---------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
class SDM {
  public:
#ifdef USE_HARDWARESERIAL
    SDM(HardwareSerial& serial, long baud = SDM_UART_BAUD, int dere_pin = DERE_PIN, int config = SDM_UART_CONFIG, bool swapuart = SWAPHWSERIAL);
#else
    SDM(ESPeasySerial& serial, long baud = SDM_UART_BAUD, int dere_pin = DERE_PIN);
#endif
    virtual ~SDM();

    void begin(void);
    float readVal(uint16_t reg, uint8_t node = SDM_B_01);                       //read value from register = reg and from deviceId = node
    uint16_t getErrCode(bool _clear = false);                                   //return last errorcode (optional clear this value, default false)
    uint16_t getErrCount(bool _clear = false);                                  //return total errors count (optional clear this value, default false)
    uint16_t getSuccCount(bool _clear = false);                                 //return total success count (optional clear this value, default false)
    void clearErrCode();                                                        //clear last errorcode
    void clearErrCount();                                                       //clear total errors count
    void clearSuccCount();                                                      //clear total success count

  private:
#ifdef USE_HARDWARESERIAL
    HardwareSerial& sdmSer;
#else
    ESPeasySerial& sdmSer;
#endif

#ifdef USE_HARDWARESERIAL
    int _config = SDM_UART_CONFIG;
    bool _swapuart = SWAPHWSERIAL;
#endif
    long _baud = SDM_UART_BAUD;
    int _dere_pin = DERE_PIN;
    uint16_t readingerrcode = SDM_ERR_NO_ERROR;                                 //4 = timeout; 3 = not enough bytes; 2 = number of bytes OK but bytes b0,b1 or b2 wrong, 1 = crc error
    uint16_t readingerrcount = 0;                                               //total errors couter
    uint32_t readingsuccesscount = 0;
    uint16_t calculateCRC(uint8_t *array, uint8_t num);
};
#endif //SDM_h
