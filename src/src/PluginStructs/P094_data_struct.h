#ifndef PLUGINSTRUCTS_P094_DATA_STRUCT_H
#define PLUGINSTRUCTS_P094_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P094

#include <ESPeasySerial.h>
#include <Regexp.h>


# define P094_REGEX_POS             0
# define P094_NR_CHAR_USE_POS       1
# define P094_FILTER_OFF_WINDOW_POS 2
# define P094_MATCH_TYPE_POS        3

# define P094_FIRST_FILTER_POS   10

# define P094_ITEMS_PER_FILTER   4
# define P094_AND_FILTER_BLOCK   3
# define P094_NR_FILTERS         (7 * P094_AND_FILTER_BLOCK)
# define P94_Nlines              (P094_FIRST_FILTER_POS + (P094_ITEMS_PER_FILTER * (P094_NR_FILTERS)))
# define P94_Nchars              128
# define P94_MAX_CAPTURE_INDEX   32


enum P094_Match_Type {
  P094_Regular_Match          = 0,
  P094_Regular_Match_inverted = 1,
  P094_Filter_Disabled        = 2
};
# define P094_Match_Type_NR_ELEMENTS 3

enum P094_Filter_Value_Type {
  P094_not_used      = 0,
  P094_packet_length = 1,
  P094_unknown1      = 2,
  P094_manufacturer  = 3,
  P094_serial_number = 4,
  P094_unknown2      = 5,
  P094_meter_type    = 6,
  P094_rssi          = 7,
  P094_position      = 8
};
# define P094_FILTER_VALUE_Type_NR_ELEMENTS 9

enum P094_Filter_Comp {
  P094_Equal_OR      = 0,
  P094_NotEqual_OR   = 1,
  P094_Equal_MUST    = 2,
  P094_NotEqual_MUST = 3
};

# define P094_FILTER_COMP_NR_ELEMENTS 4


struct P094_data_struct : public PluginTaskData_base {
public:

  P094_data_struct();

  ~P094_data_struct();

  void reset();

  bool init(ESPEasySerialPort port, 
            const int16_t serial_rx,
            const int16_t serial_tx,
            unsigned long baudrate);

  void post_init();

  bool isInitialized() const;

  void sendString(const String& data);

  bool loop();

  const String& peekSentence() const;

  void getSentence(String& string, bool appendSysTime);

  void getSentencesReceived(uint32_t& succes,
                            uint32_t& error,
                            uint32_t& length_last) const;

  void setMaxLength(uint16_t maxlenght);

  void setLine(uint8_t          varNr,
               const String& line);


  uint32_t        getFilterOffWindowTime() const;

  P094_Match_Type getMatchType() const;

  bool            invertMatch() const;

  bool            filterUsed(uint8_t lineNr) const;

  String          getFilter(uint8_t                 lineNr,
                            P094_Filter_Value_Type& capture,
                            uint32_t              & optional,
                            P094_Filter_Comp      & comparator) const;

  void          setDisableFilterWindowTimer();

  bool          disableFilterWindowActive() const;

  bool          parsePacket(const String& received) const;

  static const __FlashStringHelper * MatchType_toString(P094_Match_Type matchType);
  static const __FlashStringHelper * P094_FilterValueType_toString(P094_Filter_Value_Type valueType);
  static const __FlashStringHelper * P094_FilterComp_toString(P094_Filter_Comp comparator);


  // Made public so we don't have to copy the values when loading/saving.
  String _lines[P94_Nlines];

  static size_t P094_Get_filter_base_index(size_t filterLine);

  // Get (and increment) debug counter
  uint32_t getDebugCounter();

private:

  bool max_length_reached() const;

  ESPeasySerial *easySerial = nullptr;
  String         sentence_part;
  uint16_t       max_length               = 550;
  uint32_t       sentences_received       = 0;
  uint32_t       sentences_received_error = 0;
  bool           current_sentence_errored = false;
  uint32_t       length_last_received     = 0;
  unsigned long  disable_filter_window    = 0;
  uint32_t       debug_counter            = 0;

  bool                   valueType_used[P094_FILTER_VALUE_Type_NR_ELEMENTS];
  P094_Filter_Value_Type valueType_index[P094_NR_FILTERS];
  P094_Filter_Comp       filter_comp[P094_NR_FILTERS];
};


#endif // USES_P094

#endif // PLUGINSTRUCTS_P094_DATA_STRUCT_H