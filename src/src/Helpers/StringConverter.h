#ifndef HELPERS_STRINGCONVERTER_H
#define HELPERS_STRINGCONVERTER_H

#include <Arduino.h>

#include "../Globals/Plugins.h"
#include "../Globals/CPlugins.h"

#include "Convert.h"

class IPAddress;

// -V::569

/********************************************************************************************\
   Convert a char string to integer
 \*********************************************************************************************/

// FIXME: change original code so it uses String and String.toInt()
unsigned long str2int(const char *string);

String        ull2String(uint64_t value,
                         uint8_t  base = 10);

/********************************************************************************************\
   Check if valid float and convert string to float.
 \*********************************************************************************************/
bool string2float(const String& string,
                  float       & floatvalue);


/********************************************************************************************\
   Convert a char string to IP uint8_t array
 \*********************************************************************************************/
bool isIP(const String& string);

bool str2ip(const String& string,
               uint8_t         *IP);

bool str2ip(const char *string,
               uint8_t       *IP);

String  formatIP(const IPAddress& ip);


/********************************************************************************************\
   Handling HEX strings
 \*********************************************************************************************/

// Convert max. 8 hex decimals to unsigned long
unsigned long hexToUL(const String& input_c,
                      size_t        nrHexDecimals);

unsigned long hexToUL(const String& input_c);

unsigned long hexToUL(const String& input_c,
                      size_t        startpos,
                      size_t        nrHexDecimals);

// Convert max. 16 hex decimals to unsigned long long
unsigned long long hexToULL(const String& input_c,
                            size_t        nrHexDecimals); 

unsigned long long hexToULL(const String& input_c);

unsigned long long hexToULL(const String& input_c,
                            size_t        startpos,
                            size_t        nrHexDecimals);

String formatToHex(unsigned long value,
                   const __FlashStringHelper * prefix);

String formatToHex(unsigned long value);

String formatHumanReadable(unsigned long value,
                           unsigned long factor);

String formatHumanReadable(unsigned long value,
                           unsigned long factor,
                           int           NrDecimals);

String formatToHex_decimal(unsigned long value);

String formatToHex_decimal(unsigned long value,
                           unsigned long factor);

const __FlashStringHelper * boolToString(bool value);

/*********************************************************************************************\
   Typical string replace functions.
\*********************************************************************************************/
void   removeExtraNewLine(String& line);

void   addNewLine(String& line);

/*********************************************************************************************\
   Format a value to the set number of decimals
\*********************************************************************************************/
String doFormatUserVar(struct EventStruct *event,
                       uint8_t                rel_index,
                       bool                mustCheck,
                       bool              & isvalid);

String formatUserVarNoCheck(taskIndex_t TaskIndex,
                            uint8_t        rel_index);

String formatUserVar(taskIndex_t TaskIndex,
                     uint8_t        rel_index,
                     bool      & isvalid);

String formatUserVarNoCheck(struct EventStruct *event,
                            uint8_t                rel_index);

String formatUserVar(struct EventStruct *event,
                     uint8_t                rel_index,
                     bool              & isvalid);


String get_formatted_Controller_number(cpluginID_t cpluginID);

/*********************************************************************************************\
   Wrap a string with given pre- and postfix string.
\*********************************************************************************************/
String wrap_String(const String& string,
                   char wrap);
                   
void   wrap_String(const String& string,
                   const String& wrap,
                   String      & result);

String wrapIfContains(const String& value,
                      char          contains,
                      char          wrap = '\"');

/*********************************************************************************************\
   Format an object value pair for use in JSON.
\*********************************************************************************************/
String to_json_object_value(const __FlashStringHelper * object,
                            const __FlashStringHelper * value,
                            bool wrapInQuotes = false);

String to_json_object_value(const __FlashStringHelper * object,
                            const String& value,
                            bool wrapInQuotes = false);

String to_json_object_value(const String& object,
                            const String& value,
                            bool wrapInQuotes = false);

/*********************************************************************************************\
   Strip wrapping chars (e.g. quotes)
\*********************************************************************************************/
String stripWrappingChar(const String& text,
                         char          wrappingChar);

bool   stringWrappedWithChar(const String& text,
                             char          wrappingChar);

bool   isQuoteChar(char c);

bool   isParameterSeparatorChar(char c);

String stripQuotes(const String& text);

bool   safe_strncpy(char         *dest,
                    const __FlashStringHelper * source,
                    size_t        max_size);

bool   safe_strncpy(char         *dest,
                    const String& source,
                    size_t        max_size);

bool safe_strncpy(char       *dest,
                  const char *source,
                  size_t      max_size);

// Convert a string to lower case and replace spaces with underscores.
String to_internal_string(const String& input,
                          char          replaceSpace);

/*********************************************************************************************\
   Parse a string and get the xth command or parameter
   IndexFind = 1 => command.
    // FIXME TD-er: parseString* should use index starting at 0.
\*********************************************************************************************/
String parseString(const String& string,
                   uint8_t          indexFind,
                   char          separator = ',');

String parseStringKeepCase(const String& string,
                           uint8_t          indexFind,
                           char          separator = ',');

String parseStringToEnd(const String& string,
                        uint8_t          indexFind,
                        char          separator = ',');

String parseStringToEndKeepCase(const String& string,
                                uint8_t          indexFind,
                                char          separator = ',');

String tolerantParseStringKeepCase(const String& string,
                                   uint8_t          indexFind,
                                   char          separator = ',');

// escapes special characters in strings for use in html-forms
bool   htmlEscapeChar(char    c,
                      String& esc);

void   htmlEscape(String& html,
                  char    c);

void   htmlEscape(String& html);

void   htmlStrongEscape(String& html);

String URLEncode(const char *msg);

void   repl(const __FlashStringHelper * key,
            const String& val,
            String      & s,
            bool       useURLencode);

void   repl(const __FlashStringHelper * key,
            const char* val,
            String      & s,
            bool       useURLencode);

void   repl(const String& key,
            const String& val,
            String      & s,
            bool       useURLencode);

#ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
void parseSpecialCharacters(String& s,
                            bool useURLencode);
#endif // ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER

/********************************************************************************************\
   replace other system variables like %sysname%, %systime%, %ip%
 \*********************************************************************************************/
void parseControllerVariables(String            & s,
                              struct EventStruct *event,
                              bool             useURLencode);

void parseSingleControllerVariable(String            & s,
                                   struct EventStruct *event,
                                   uint8_t                taskValueIndex,
                                   bool             useURLencode);

void parseSystemVariables(String& s,
                          bool useURLencode);

void parseEventVariables(String            & s,
                         struct EventStruct *event,
                         bool             useURLencode);

bool getConvertArgument(const __FlashStringHelper * marker,
                        const String& s,
                        float       & argument,
                        int         & startIndex,
                        int         & endIndex);

bool getConvertArgument2(const __FlashStringHelper * marker,
                         const String& s,
                         float       & arg1,
                         float       & arg2,
                         int         & startIndex,
                         int         & endIndex);

bool getConvertArgumentString(const __FlashStringHelper * marker,
                              const String& s,
                              String      & argumentString,
                              int         & startIndex,
                              int         & endIndex);

bool getConvertArgumentString(const String& marker,
                              const String& s,
                              String      & argumentString,
                              int         & startIndex,
                              int         & endIndex);

// Parse conversions marked with "%conv_marker%(float)"
// Must be called last, since all sensor values must be converted, processed, etc.
void parseStandardConversions(String& s,
                              bool useURLencode);


bool HasArgv(const char  *string,
             unsigned int argc);

bool GetArgv(const char  *string,
             String     & argvString,
             unsigned int argc,
             char         separator = ',');

bool GetArgvBeginEnd(const char        *string,
                     const unsigned int argc,
                     int              & pos_begin,
                     int              & pos_end,
                     char               separator = ',');


#endif // HELPERS_STRINGCONVERTER_H
