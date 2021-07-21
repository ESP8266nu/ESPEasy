#ifndef WEBSERVER_WEBSERVER_MARKUP_FORMS_H
#define WEBSERVER_WEBSERVER_MARKUP_FORMS_H

#include "../WebServer/common.h"

#include "../DataStructs/MAC_address.h"
#include "../Globals/Plugins.h"
#include "../Helpers/StringGenerator_GPIO.h"


// ********************************************************************************
// Add a separator as row start
// ********************************************************************************
void addFormSeparator(int clspan);

// ********************************************************************************
// Add a note as row start
// ********************************************************************************
void addFormNote(const __FlashStringHelper * text);
void addFormNote(const String& text, const String& id = "");

// ********************************************************************************
// Create Forms
// ********************************************************************************


// ********************************************************************************
// Add a checkbox Form
// ********************************************************************************

void addFormCheckBox_disabled(const String& label,
                              const String& id,
                              boolean       checked
                              #ifdef        ENABLE_TOOLTIPS
                              ,
                              const String& tooltip = EMPTY_STRING
                              #endif // ifdef ENABLE_TOOLTIPS
                              );

void addFormCheckBox(const String& label,
                     const String& id,
                     boolean       checked,
                     bool          disabled = false
                     #ifdef        ENABLE_TOOLTIPS
                     ,
                     const String& tooltip = EMPTY_STRING
                     #endif // ifdef ENABLE_TOOLTIPS
                     );

void addFormCheckBox(LabelType::Enum label,
                     boolean         checked,
                     bool            disabled = false
                     #ifdef          ENABLE_TOOLTIPS
                     ,
                     const String  & tooltip = EMPTY_STRING
                     #endif // ifdef ENABLE_TOOLTIPS
                     );

void addFormCheckBox_disabled(LabelType::Enum label,
                              boolean         checked);
void addFormCheckBox(const __FlashStringHelper * label, const __FlashStringHelper * id, boolean checked, bool disabled = false);
void addFormCheckBox(const __FlashStringHelper * label, const String& id, boolean checked, bool disabled = false);

// ********************************************************************************
// Add a Numeric Box form
// ********************************************************************************
void addFormNumericBox(LabelType::Enum label,
                       int             value,
                       int             min = INT_MIN,
                       int             max = INT_MAX
                       #ifdef          ENABLE_TOOLTIPS
                       ,
                       const String  & tooltip = EMPTY_STRING
                       #endif // ifdef ENABLE_TOOLTIPS
                       );

void addFormNumericBox(const String& label,
                       const String& id,
                       int           value,
                       int           min = INT_MIN,
                       int           max = INT_MAX
                       #ifdef        ENABLE_TOOLTIPS
                       ,
                       const String& tooltip = EMPTY_STRING
                       #endif // ifdef ENABLE_TOOLTIPS
                       );

void addFormFloatNumberBox(LabelType::Enum label,
                           float           value,
                           float           min,
                           float           max,
                           uint8_t         nrDecimals = 6,
                           float           stepsize   = 0.0f
                           #ifdef ENABLE_TOOLTIPS
                           ,
                           const String& tooltip = EMPTY_STRING
                           #endif // ifdef ENABLE_TOOLTIPS
                           );

void addFormFloatNumberBox(const String& label,
                           const String& id,
                           float         value,
                           float         min,
                           float         max,
                           uint8_t       nrDecimals = 6,
                           float         stepsize   = 0.0f
                           #ifdef ENABLE_TOOLTIPS
                           ,
                           const String& tooltip = EMPTY_STRING
                           #endif // ifdef ENABLE_TOOLTIPS
                           );
void addFormNumericBox(const __FlashStringHelper * label, const __FlashStringHelper * id, int value, int min = INT_MIN, int max = INT_MAX);


// ********************************************************************************
// Add a task selector form
// ********************************************************************************
void addTaskSelectBox(const String& label,
                      const String& id,
                      taskIndex_t   choice);

// ********************************************************************************
// Add a Text Box form
// ********************************************************************************
void addFormTextBox(const __FlashStringHelper * label,
                    const __FlashStringHelper * id,
                    const String& value,
                    int           maxlength,
                    bool          readonly = false,
                    bool          required = false,
                    const String& pattern = "");

void addFormTextBox(const String& label,
                    const String& id,
                    const String& value,
                    int           maxlength,
                    bool          readonly = false,
                    bool          required = false,
                    const String& pattern  = ""
                    #ifdef ENABLE_TOOLTIPS
                    ,
                    const String& tooltip = EMPTY_STRING
                    #endif // ifdef ENABLE_TOOLTIPS
                    );


void addFormTextArea(const String& label,
                     const String& id,
                     const String& value,
                     int           maxlength,
                     int           rows,
                     int           columns,
                     bool          readonly = false,
                     bool          required = false
                     #ifdef        ENABLE_TOOLTIPS
                     ,
                     const String& tooltip = EMPTY_STRING
                     #endif // ifdef ENABLE_TOOLTIPS
                     );

// ********************************************************************************
// Add a Password Box form
// ********************************************************************************

void addFormPasswordBox(const String& label,
                        const String& id,
                        const String& password,
                        int           maxlength
                        #ifdef        ENABLE_TOOLTIPS
                        ,
                        const String& tooltip = EMPTY_STRING
                        #endif // ifdef ENABLE_TOOLTIPS
                        );

bool getFormPassword(const String& id,
                     String      & password);

// ********************************************************************************
// Add a IP Box form
// ********************************************************************************

void addFormIPBox(const String& label,
                  const String& id,
                  const uint8_t ip[4]);

// ********************************************************************************
// Add a MAC address Box form
// ********************************************************************************
void addFormMACBox(const String& label, const String& id, const MAC_address mac);

// ********************************************************************************
// Add a IP Access Control select dropdown list
// ********************************************************************************
void addFormIPaccessControlSelect(const String& label,
                                  const String& id,
                                  int           choice);

// ********************************************************************************
// Add a selector form
// ********************************************************************************

void addFormPinSelect(const String& label,
                      const String& id,
                      int           choice);
void addFormPinSelect(const String& label,
                      const __FlashStringHelper * id,
                      int           choice);
void addFormPinSelect(PinSelectPurpose purpose, const String& label, const __FlashStringHelper * id, int choice);

void addFormPinSelectI2C(const String& label,
                         const String& id,
                         int           choice);

void addFormSelectorI2C(const String& id,
                        int           addressCount,
                        const int     addresses[],
                        int           selectedIndex
                        #ifdef ENABLE_TOOLTIPS
                        ,
                        const String& tooltip = EMPTY_STRING
                        #endif
                        );

void addFormSelector(const String& label,
                     const String& id,
                     int           optionCount,
                     const String  options[],
                     const int     indices[],
                     int           selectedIndex
                     #ifdef ENABLE_TOOLTIPS
                     ,
                     const String& tooltip = EMPTY_STRING
                     #endif
                     );

void addFormSelector(const __FlashStringHelper * label, const __FlashStringHelper * id, int optionCount, const __FlashStringHelper * options[], const int indices[], int selectedIndex, bool reloadonchange = false);
void addFormSelector(const String& label, const String& id, int optionCount, const __FlashStringHelper * options[], const int indices[], int selectedIndex);
void addFormSelector(const __FlashStringHelper * label, const __FlashStringHelper * id, int optionCount, const String options[], const int indices[], int selectedIndex);

void addFormSelector(const String& label,
                     const String& id,
                     int           optionCount,
                     const __FlashStringHelper * options[],
                     const int     indices[],
                     int           selectedIndex,
                     bool          reloadonchange);

void addFormSelector(const String& label,
                     const String& id,
                     int           optionCount,
                     const __FlashStringHelper * options[],
                     const int     indices[],
                     const String  attr[],
                     int           selectedIndex,
                     boolean       reloadonchange);

void addFormSelector_script(const String& label,
                            const String& id,
                            int           optionCount,
                            const __FlashStringHelper * options[],
                            const int     indices[],
                            const String  attr[],
                            int           selectedIndex,
                            const String& onChangeCall);


void addFormSelector(const String& label,
                     const String& id,
                     int           optionCount,
                     const String  options[],
                     const int     indices[],
                     int           selectedIndex,
                     bool          reloadonchange);

void addFormSelector(const String& label,
                     const String& id,
                     int           optionCount,
                     const String  options[],
                     const int     indices[],
                     const String  attr[],
                     int           selectedIndex,
                     boolean       reloadonchange
                     #ifdef ENABLE_TOOLTIPS
                     ,
                     const String& tooltip = EMPTY_STRING
                     #endif
                     );

void addFormSelector_script(const String& label,
                            const String& id,
                            int           optionCount,
                            const String  options[],
                            const int     indices[],
                            const String  attr[],
                            int           selectedIndex,
                            const String& onChangeCall
                            #ifdef ENABLE_TOOLTIPS
                            ,
                            const String& tooltip = EMPTY_STRING
                            #endif
                            );

// ********************************************************************************
// Add a GPIO pin select dropdown list
// ********************************************************************************
void addFormPinStateSelect(int gpio,
                           int choice);

// ********************************************************************************
// Retrieve return values from form/checkbox.
// ********************************************************************************


int getFormItemInt(const __FlashStringHelper * key, int defaultValue);
int getFormItemInt(const String& key, int defaultValue);

bool getCheckWebserverArg_int(const String& key,
                              int         & value);

bool update_whenset_FormItemInt(const String& key,
                                int         & value);

bool update_whenset_FormItemInt(const String& key,
                                uint8_t     & value);

// Note: Checkbox values will not appear in POST Form data if unchecked.
// So if webserver does not have an argument for a checkbox form, it means it should be considered unchecked.
bool isFormItemChecked(const __FlashStringHelper * id);
bool isFormItemChecked(const String& id);
bool isFormItemChecked(const LabelType::Enum& id);

int getFormItemInt(const __FlashStringHelper * id);
int getFormItemInt(const String& id);
int getFormItemInt(const LabelType::Enum& id);

float getFormItemFloat(const __FlashStringHelper * id);
float getFormItemFloat(const String& id);
float getFormItemFloat(const LabelType::Enum& id);

bool  isFormItem(const String& id);

void  copyFormPassword(const String& id,
                       char         *pPassword,
                       int           maxlength);


#endif // ifndef WEBSERVER_WEBSERVER_MARKUP_FORMS_H
