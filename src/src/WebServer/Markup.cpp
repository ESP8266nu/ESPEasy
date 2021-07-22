
#include "../WebServer/Markup.h"

#include "../WebServer/HTML_wrappers.h"

#include "../Globals/Settings.h"

#include "../Helpers/Hardware.h"
#include "../Helpers/StringGenerator_GPIO.h"

#include "../../ESPEasy_common.h"

// ********************************************************************************
// Add Selector
// ********************************************************************************
void addSelector(const String             & id,
                 int                        optionCount,
                 const __FlashStringHelper *options[],
                 const int                  indices[],
                 const String               attr[],
                 int                        selectedIndex)
{
  addSelector(id, optionCount, options, indices, attr, selectedIndex, false, true, F("wide"));
}

void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex)
{
  addSelector(id, optionCount, options, indices, attr, selectedIndex, false, true, F("wide"));
}

void addSelector(const String             & id,
                 int                        optionCount,
                 const __FlashStringHelper *options[],
                 const int                  indices[],
                 const String               attr[],
                 int                        selectedIndex,
                 boolean                    reloadonchange,
                 bool                       enabled)
{
  addSelector(id, optionCount, options, indices, attr, selectedIndex, reloadonchange, enabled, F("wide"));
}

void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 boolean       reloadonchange,
                 bool          enabled)
{
  addSelector(id, optionCount, options, indices, attr, selectedIndex, reloadonchange, enabled, F("wide"));
}

void addSelector(const String             & id,
                 int                        optionCount,
                 const __FlashStringHelper *options[],
                 const int                  indices[],
                 const String               attr[],
                 int                        selectedIndex,
                 boolean                    reloadonchange,
                 bool                       enabled,
                 const String& classname
                 #ifdef                     ENABLE_TOOLTIPS
                 , const String           & tooltip
                 #endif // ifdef ENABLE_TOOLTIPS
                 )
{
  // FIXME TD-er Change boolean to disabled
  if (reloadonchange)
  {
    addSelector_Head_reloadOnChange(id, classname, !enabled
                                    #ifdef ENABLE_TOOLTIPS
                                    , tooltip
                                    #endif // ifdef ENABLE_TOOLTIPS
                                    );
  } else {
    do_addSelector_Head(id, classname, EMPTY_STRING, !enabled
                        #ifdef ENABLE_TOOLTIPS
                        , tooltip
                        #endif // ifdef ENABLE_TOOLTIPS
                        );
  }
  addSelector_options(optionCount, options, indices, attr, selectedIndex);
  addSelector_Foot();
}

void addSelector(const String  & id,
                 int             optionCount,
                 const String    options[],
                 const int       indices[],
                 const String    attr[],
                 int             selectedIndex,
                 boolean         reloadonchange,
                 bool            enabled,
                 const String& classname
                 #ifdef          ENABLE_TOOLTIPS
                 , const String& tooltip
                 #endif // ifdef ENABLE_TOOLTIPS
                 )
{
  // FIXME TD-er Change boolean to disabled
  if (reloadonchange)
  {
    addSelector_Head_reloadOnChange(id, classname, !enabled
                                    #ifdef ENABLE_TOOLTIPS
                                    , tooltip
                                    #endif // ifdef ENABLE_TOOLTIPS
                                    );
  } else {
    do_addSelector_Head(id, classname, EMPTY_STRING, !enabled
                        #ifdef ENABLE_TOOLTIPS
                        , tooltip
                        #endif // ifdef ENABLE_TOOLTIPS
                        );
  }
  addSelector_options(optionCount, options, indices, attr, selectedIndex);
  addSelector_Foot();
}

void addSelector_options(int optionCount, const __FlashStringHelper *options[], const int indices[], const String attr[], int selectedIndex)
{
  int index;

  for (uint8_t x = 0; x < optionCount; x++)
  {
    if (indices) {
      index = indices[x];
    }
    else {
      index = x;
    }
    String attr_str;

    if (attr)
    {
      attr_str = attr[x];
    }
    addSelector_Item(options[x], index, selectedIndex == index, false, attr_str);
    if (x % 10 == 0) delay(0);
  }
}

void addSelector_options(int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex)
{
  int index;

  for (uint8_t x = 0; x < optionCount; x++)
  {
    if (indices) {
      index = indices[x];
    }
    else {
      index = x;
    }
    String attr_str;

    if (attr)
    {
      attr_str = attr[x];
    }
    addSelector_Item(options[x], index, selectedIndex == index, false, attr_str);
    if (x % 10 == 0) delay(0);
  }
}

void addSelector_Head(const String& id) {
  do_addSelector_Head(id, F("wide"), EMPTY_STRING, false
                      #ifdef ENABLE_TOOLTIPS
                      , F("")
                      #endif // ifdef ENABLE_TOOLTIPS
                      );
}

void addSelector_Head_reloadOnChange(const String& id) {
  addSelector_Head_reloadOnChange(id, F("wide"), false);
}

void addSelector_Head_reloadOnChange(const String& id, const String& classname, bool disabled
                                     #ifdef ENABLE_TOOLTIPS
                                     , const String& tooltip
                                     #endif // ifdef ENABLE_TOOLTIPS
                                     ) {
  do_addSelector_Head(id, classname, F("return dept_onchange(frmselect)"), disabled
                      #ifdef ENABLE_TOOLTIPS
                      , tooltip
                      #endif // ifdef ENABLE_TOOLTIPS
                      );
}

void addSelector_Head_reloadOnChange(const String& id, const String& classname, const String& onChangeCall, bool disabled
                                     #ifdef ENABLE_TOOLTIPS
                                     , const String& tooltip
                                     #endif // ifdef ENABLE_TOOLTIPS
                                     ) {
  do_addSelector_Head(id, classname, onChangeCall, disabled
                      #ifdef ENABLE_TOOLTIPS
                      , tooltip
                      #endif // ifdef ENABLE_TOOLTIPS
                      );
}

void do_addSelector_Head(const String& id, const String& classname, const String& onChangeCall, const bool& disabled
                         #ifdef ENABLE_TOOLTIPS
                         , const String& tooltip
                         #endif // ifdef ENABLE_TOOLTIPS
                         )
{
  addHtml(F("<select "));
  addHtmlAttribute(F("class"), classname);
  addHtmlAttribute(F("name"),  id);
  addHtmlAttribute(F("id"),    id);

  #ifdef ENABLE_TOOLTIPS

  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
  #endif // ifdef ENABLE_TOOLTIPS

  if (disabled) {
    addDisabled();
  }

  if (onChangeCall.length() > 0) {
    addHtmlAttribute(F("onchange"), onChangeCall);
  }
  addHtml('>');
}

void addPinSelector_Item(PinSelectPurpose purpose, const String& gpio_label, int gpio, boolean selected, boolean disabled, const String& attr)
{
  if (gpio != -1) // empty selection can never be disabled...
  {
    int  pinnr = -1;
    bool input, output, warning;

    if (getGpioInfo(gpio, pinnr, input, output, warning)) {
      bool includeI2C = true;
      bool includeSPI = true;
        #ifdef HAS_ETHERNET
      bool includeEthernet = true;
        #endif // ifdef HAS_ETHERNET

      switch (purpose) {
        case PinSelectPurpose::SPI:
          includeSPI = false;
          break;
        case PinSelectPurpose::Ethernet:
            #ifdef HAS_ETHERNET
          includeEthernet = false;
            #endif // ifdef HAS_ETHERNET
          break;
        case PinSelectPurpose::Generic:

          if (!input && !output) {
            return;
          }
          break;

        case PinSelectPurpose::Generic_input:

          if (!input) {
            return;
          }
          break;

        case PinSelectPurpose::Generic_output:

          if (!output) {
            return;
          }
          break;

        case PinSelectPurpose::Generic_bidir:
        case PinSelectPurpose::I2C:
          includeI2C = false;

          if (!output || !input) {
            // SDA is obviously bidirectional.
            // SCL is obviously output, but can be held down by a slave device to signal clock stretch limit.
            // Thus both must be capable of input & output.
            return;
          }
          break;
      }

      if (includeI2C && Settings.isI2C_pin(gpio)) {
        disabled = true;
      }

      if (Settings.UseSerial && ((gpio == 1) || (gpio == 3))) {
        disabled = true;
      }

      if (includeSPI && Settings.isSPI_pin(gpio)) {
        disabled = true;
      }

  #ifdef HAS_ETHERNET

      if (Settings.isEthernetPin(gpio) || (includeEthernet && Settings.isEthernetPinOptional(gpio))) {
        disabled = true;
      }
  #endif // ifdef HAS_ETHERNET
    }
  }

  addSelector_Item(gpio_label,
                   gpio,
                   selected,
                   disabled);
}

void addSelector_Item(const __FlashStringHelper *option, int index, boolean selected, boolean disabled, const String& attr)
{
  addHtml(F("<option "));
  addHtmlAttribute(F("value"), index);

  if (selected) {
    addHtml(F(" selected"));
  }

  if (disabled) {
    addDisabled();
  }

  if (attr.length() > 0)
  {
    addHtml(' ');
    addHtml(attr);
  }
  addHtml('>');
  addHtml(option);
  addHtml(F("</option>"));
}

void addSelector_Item(const String& option, int index, boolean selected, boolean disabled, const String& attr)
{
  addHtml(F("<option "));
  addHtmlAttribute(F("value"), index);

  if (selected) {
    addHtml(F(" selected"));
  }

  if (disabled) {
    addDisabled();
  }

  if (attr.length() > 0)
  {
    addHtml(' ');
    addHtml(attr);
  }
  addHtml('>');
  addHtml(option);
  addHtml(F("</option>"));
}

void addSelector_Foot()
{
  addHtml(F("</select>"));
}

void addUnit(const __FlashStringHelper *unit)
{
  addHtml(F(" ["));
  addHtml(unit);
  addHtml(']');
}

void addUnit(const String& unit)
{
  addHtml(F(" ["));
  addHtml(unit);
  addHtml(']');
}

void addUnit(char unit)
{
  addHtml(F(" ["));
  addHtml(unit);
  addHtml(']');
}

void addRowLabel_tr_id(const __FlashStringHelper *label, const __FlashStringHelper *id)
{
  addRowLabel_tr_id(String(label), String(id));
}

void addRowLabel_tr_id(const __FlashStringHelper *label, const String& id)
{
  addRowLabel_tr_id(String(label), id);
}

void addRowLabel_tr_id(const String& label, const String& id)
{
  String tr_id = F("tr_");

  tr_id += id;
  addRowLabel(label, tr_id);
}

void addRowLabel(const __FlashStringHelper *label)
{
  html_TR_TD();
  addHtml(label);
  addHtml(':');
  addHtml(F("</td>"));
  html_TD();
}

void addRowLabel(const String& label, const String& id)
{
  if (id.length() > 0) {
    addHtml(F("<TR id='"));
    addHtml(id);
    addHtml(F("'><TD>"));
  } else {
    html_TR_TD();
  }

  if (!label.isEmpty()) {
    addHtml(label);
    addHtml(':');
  }
  addHtml(F("</td>"));
  html_TD();
}

// Add a row label and mark it with copy markers to copy it to clipboard.
void addRowLabel_copy(const __FlashStringHelper *label) {
  addHtml(F("<TR>"));
  html_copyText_TD();
  addHtml(label);
  addHtml(':');
  html_copyText_marker();
  html_copyText_TD();
}

void addRowLabel_copy(const String& label) {
  addHtml(F("<TR>"));
  html_copyText_TD();
  addHtml(label);
  addHtml(':');
  html_copyText_marker();
  html_copyText_TD();
}

void addRowLabel(LabelType::Enum label) {
  addRowLabel(getLabel(label));
}

void addRowLabelValue(LabelType::Enum label) {
  addRowLabel(getLabel(label));
  addHtml(getValue(label));
}

void addRowLabelValue_copy(LabelType::Enum label) {
  addRowLabel_copy(getLabel(label));
  addHtml(getValue(label));
}

// ********************************************************************************
// Add a header
// ********************************************************************************
void addTableSeparator(const __FlashStringHelper *label, int colspan, int h_size)
{
  addHtml(F("<TR><TD colspan="));
  addHtmlInt(colspan);
  addHtml(F("><H"));
  addHtmlInt(h_size);
  addHtml('>');
  addHtml(label);
  addHtml(F("</H"));
  addHtmlInt(h_size);
  addHtml(F("></TD></TR>"));
}

void addTableSeparator(const __FlashStringHelper *label, int colspan, int h_size, const __FlashStringHelper *helpButton)
{
  addTableSeparator(String(label), colspan, h_size, String(helpButton));
}

void addTableSeparator(const String& label, int colspan, int h_size, const String& helpButton) {
  {
    String html;
    html.reserve(32 + label.length());
    html += F("<TR><TD colspan=");
    html += colspan;
    html += F("><H");
    html += h_size;
    html += '>';
    html += label;
    addHtml(html);
  }

  if (helpButton.length() > 0) {
    addHelpButton(helpButton);
  }
  {
    String html;
    html.reserve(16);
    html += F("</H");
    html += h_size;
    html += F("></TD></TR>");
    addHtml(html);
  }
}

void addFormHeader(const __FlashStringHelper *header) {
  html_TR();
  html_table_header(header, EMPTY_STRING, EMPTY_STRING, 225);
  html_table_header(F(""));
}

void addFormHeader(const String& header, const String& helpButton) {
  addFormHeader(header, helpButton, EMPTY_STRING);
}

void addFormHeader(const String& header, const String& helpButton, const String& rtdHelpButton)
{
  html_TR();
  html_table_header(header, helpButton, rtdHelpButton, 225);
  html_table_header(F(""));
}

// ********************************************************************************
// Add a sub header
// ********************************************************************************
void addFormSubHeader(const __FlashStringHelper *header) {
  addTableSeparator(header, 2, 3);
}

void addFormSubHeader(const String& header)
{
  addTableSeparator(header, 2, 3);
}

// ********************************************************************************
// Add a checkbox
// ********************************************************************************
void addCheckBox(const __FlashStringHelper *id, boolean checked, bool disabled)
{
  addCheckBox(String(id), checked, disabled);
}

void addCheckBox(const String& id, boolean checked, bool disabled
                 #ifdef ENABLE_TOOLTIPS
                 , const String& tooltip
                 #endif // ifdef ENABLE_TOOLTIPS
                 )
{
  addHtml(F("<label class='container'>&nbsp;"));
  addHtml(F("<input "));
  addHtmlAttribute(F("type"), F("checkbox"));
  addHtmlAttribute(F("id"),   id);
  addHtmlAttribute(F("name"), id);

  if (checked) {
    addHtml(F(" checked"));
  }

  if (disabled) { addDisabled(); }
  addHtml(F("><span class='checkmark"));

  if (disabled) { addDisabled(); }
  addHtml('\'');
  #ifdef ENABLE_TOOLTIPS

  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
  #endif // ifdef ENABLE_TOOLTIPS
  addHtml(F("></span></label>"));
}

// ********************************************************************************
// Add a numeric box
// ********************************************************************************
void addNumericBox(const __FlashStringHelper *id, int value, int min, int max)
{
  addNumericBox(String(id), value, min, max);
}

void addNumericBox(const String& id, int value, int min, int max
                   #ifdef ENABLE_TOOLTIPS
                   , const String& classname, const String& tooltip
                   #endif // ifdef ENABLE_TOOLTIPS
                   )
{
  addHtml(F("<input "));
  #ifdef ENABLE_TOOLTIPS
  addHtmlAttribute(F("class"), classname);
  #else // ifdef ENABLE_TOOLTIPS
  addHtmlAttribute(F("class"), F("widenumber"));
  #endif  // ifdef ENABLE_TOOLTIPS
  addHtmlAttribute(F("type"),  F("number"));
  addHtmlAttribute(F("name"),  id);

  #ifdef ENABLE_TOOLTIPS

  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
  #endif // ifdef ENABLE_TOOLTIPS

  if (value < min) {
    value = min;
  }

  if (value > max) {
    value = max;
  }

  if (min != INT_MIN)
  {
    addHtmlAttribute(F("min"), min);
  }

  if (max != INT_MAX)
  {
    addHtmlAttribute(F("max"), max);
  }
  addHtmlAttribute(F("value"), value);
  addHtml('>');
}

#ifdef ENABLE_TOOLTIPS
void addNumericBox(const String& id, int value, int min, int max)
{
  addNumericBox(id, value, min, max, F("widenumber"));
}

#endif // ifdef ENABLE_TOOLTIPS

void addFloatNumberBox(const String& id, float value, float min, float max, uint8_t nrDecimals, float stepsize
                       #ifdef ENABLE_TOOLTIPS
                       , const String& tooltip
                       #endif // ifdef ENABLE_TOOLTIPS
                       )
{
  String html;

  html.reserve(64 + id.length());

  html += F("<input type='number' name='");
  html += id;
  html += '\'';
  html += F(" min=");
  html += String(min, nrDecimals);
  html += F(" max=");
  html += String(max, nrDecimals);
  html += F(" step=");

  if (stepsize <= 0.0f) {
    html += F("0.");

    for (uint8_t i = 1; i < nrDecimals; ++i) {
      html += '0';
    }
    html += '1';
  } else {
    html += String(stepsize, nrDecimals);
  }

  html += F(" style='width:7em;' value=");
  html += String(value, nrDecimals);

  #ifdef ENABLE_TOOLTIPS

  if (tooltip.length() > 0) {
    html += F("title='");
    html += tooltip;
    html += F("' ");
  }
  #endif // ifdef ENABLE_TOOLTIPS
  html += '>';

  addHtml(html);
}

// ********************************************************************************
// Add Textbox
// ********************************************************************************
void addTextBox(const String& id, const String&  value, int maxlength, bool readonly, bool required, const String& pattern) {
  addTextBox(id, value, maxlength, readonly, required, pattern, F("wide"));
}

void addTextBox(const String  & id,
                const String  & value,
                int             maxlength,
                bool            readonly,
                bool            required,
                const String  & pattern,
                const String& classname
                #ifdef          ENABLE_TOOLTIPS
                , const String& tooltip
                #endif // ifdef ENABLE_TOOLTIPS
                )
{
  addHtml(F("<input "));
  addHtmlAttribute(F("class"),     classname);
  addHtmlAttribute(F("type"),      F("text"));
  addHtmlAttribute(F("name"),      id);
  addHtmlAttribute(F("maxlength"), maxlength);
  addHtmlAttribute(F("value"),     value);

  if (readonly) {
    addHtml(F(" readonly "));
  }

  if (required) {
    addHtml(F(" required "));
  }

  if (pattern.length() > 0) {
    addHtmlAttribute(F("pattern"), pattern);
  }

  #ifdef ENABLE_TOOLTIPS

  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
  #endif // ifdef ENABLE_TOOLTIPS
  addHtml('>');
}

// ********************************************************************************
// Add Textarea
// ********************************************************************************
void addTextArea(const String  & id,
                 const String  & value,
                 int             maxlength,
                 int             rows,
                 int             columns,
                 bool            readonly,
                 bool          required
                 #ifdef          ENABLE_TOOLTIPS
                 , const String& tooltip
                 #endif // ifdef ENABLE_TOOLTIPS
                 )
{
  addHtml(F("<textarea "));
  addHtmlAttribute(F("class"),     F("wide"));
  addHtmlAttribute(F("type"),      F("text"));
  addHtmlAttribute(F("name"),      id);
  addHtmlAttribute(F("maxlength"), maxlength);
  addHtmlAttribute(F("rows"),      rows);
  addHtmlAttribute(F("cols"),      columns);

  if (readonly) {
    addHtml(F(" readonly "));
  }

  if (required) {
    addHtml(F(" required "));
  }

  #ifdef ENABLE_TOOLTIPS

  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
  #endif // ifdef ENABLE_TOOLTIPS
  addHtml('>');
  addHtml(value);
  addHtml(F("</textarea>"));
}

// ********************************************************************************
// Add Help Buttons
// ********************************************************************************

// adds a Help Button with points to the the given Wiki Subpage
// If url starts with "RTD", it will be considered as a Read-the-docs link
void addHelpButton(const __FlashStringHelper *url) {
  addHelpButton(String(url));
}

void addHelpButton(const String& url) {
#ifndef WEBPAGE_TEMPLATE_HIDE_HELP_BUTTON

  if (url.startsWith("RTD")) {
    addRTDHelpButton(url.substring(3));
  } else {
    addHelpButton(url, false);
  }
#endif // ifndef WEBPAGE_TEMPLATE_HIDE_HELP_BUTTON
}

void addRTDHelpButton(const String& url)
{
  addHelpButton(url, true);
}

void addHelpButton(const String& url, bool isRTD)
{
  #ifndef WEBPAGE_TEMPLATE_HIDE_HELP_BUTTON
  addHtmlLink(
    F("button help"),
    makeDocLink(url, isRTD),
    isRTD ? F("&#8505;") : F("&#10068;"));
  #endif // ifndef WEBPAGE_TEMPLATE_HIDE_HELP_BUTTON
}

void addRTDPluginButton(pluginID_t taskDeviceNumber) {
  String url;

  url.reserve(16);
  url = F("Plugin/P");

  if (taskDeviceNumber < 100) { url += '0'; }

  if (taskDeviceNumber < 10) { url += '0'; }
  url += String(taskDeviceNumber);
  url += F(".html");
  addRTDHelpButton(url);

  switch (taskDeviceNumber) {
    case 76:
    case 77:
      addHtmlLink(
        F("button help"),
        makeDocLink(F("Reference/Safety.html"), true),
        F("&#9889;")); // High voltage sign
      break;
  }
}

String makeDocLink(const String& url, bool isRTD) {
  String result;

  if (!url.startsWith(F("http"))) {
    if (isRTD) {
      result += F("https://espeasy.readthedocs.io/en/latest/");
    } else {
      result += F("http://www.letscontrolit.com/wiki/index.php/");
    }
  }
  result += url;
  return result;
}

void addPinSelect(PinSelectPurpose purpose, const __FlashStringHelper *id,  int choice)
{
  addPinSelect(purpose, String(id), choice);
}

void addPinSelect(PinSelectPurpose purpose, const String& id,  int choice)
{
  addSelector_Head(id);

  // At i == 0 && gpio == -1, add the "- None -" option first
  int i    = 0;
  int gpio = -1;

  while (gpio <= MAX_GPIO) {
    int  pinnr = -1;
    bool input, output, warning = false;

    // Make sure getGpioInfo is called (compiler may optimize it away if (i == 0))
    const bool UsableGPIO = getGpioInfo(gpio, pinnr, input, output, warning);

    if (UsableGPIO || (i == 0)) {
      String gpio_label = createGPIO_label(gpio, pinnr, input, output, warning);
      gpio_label += getConflictingUse_wrapped(gpio, purpose);
      addPinSelector_Item(
        purpose,
        gpio_label,
        gpio,
        choice == gpio);

      ++i;
    }
    ++gpio;
  }
  addSelector_Foot();
}

#ifdef ESP32
void addADC_PinSelect(AdcPinSelectPurpose purpose, const String& id,  int choice)
{
  addSelector_Head(id);

  // At i == 0 && gpio == -1, add the "Hall Effect" option first
  int i    = 0;
  int gpio = -1;

  if ((purpose == AdcPinSelectPurpose::ADC_Touch_HallEffect) ||
      (purpose == AdcPinSelectPurpose::ADC_Touch_Optional)) {
    addPinSelector_Item(
      PinSelectPurpose::Generic,
      purpose == AdcPinSelectPurpose::ADC_Touch_Optional ? F("- None -") : formatGpioName_ADC(gpio),
      gpio,
      choice == gpio);
    ++i;
  }

  while (i <= MAX_GPIO && gpio <= MAX_GPIO) {
    int  pinnr = -1;
    bool input, output, warning;

    if (purpose == AdcPinSelectPurpose::TouchOnly) {
      // For touch only list, sort based on touch number
      // Default sort is on GPIO number.
      gpio = touchPinToGpio(i);
    } else {
      ++gpio;
    }

    if (getGpioInfo(gpio, pinnr, input, output, warning)) {
      int adc, ch, t;

      if (getADC_gpio_info(gpio, adc, ch, t)) {
        if ((purpose != AdcPinSelectPurpose::TouchOnly) || (t >= 0)) {
          String gpio_label;
          gpio_label = formatGpioName_ADC(gpio);

          if (adc != 0) {
            gpio_label += F(" / ");
            gpio_label += createGPIO_label(gpio, pinnr, input, output, warning);
            gpio_label += getConflictingUse_wrapped(gpio);
          }
          addPinSelector_Item(
            PinSelectPurpose::Generic,
            gpio_label,
            gpio,
            choice == gpio);
        }
      }
    }
    ++i;
  }
  addSelector_Foot();
}

#endif // ifdef ESP32
