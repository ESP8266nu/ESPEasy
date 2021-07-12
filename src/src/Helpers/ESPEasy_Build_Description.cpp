#include "ESPEasy_Build_Description.h"


#include "../Globals/CPlugins.h"
#include "../Globals/NPlugins.h"
#include "../Globals/Plugins.h"

xPluginEnumerator::xPluginEnumerator() {}

void xPluginEnumerator::setSize(unsigned int maxID) {
  const unsigned int wordSize = (maxID / 16) + 1;

  if (_bitmap.size() < wordSize) {
    _bitmap.resize(wordSize, 0);
  }
}

void xPluginEnumerator::add(unsigned int ID) {
  setSize(ID);
  unsigned int wordIndex = ID / 16;
  unsigned int bitIndex  = 15 - (ID % 16);

  bitSet(_bitmap[wordIndex], bitIndex);
}

String xPluginEnumerator::getString(char separator) const {
  String result;

  result.reserve(_bitmap.size() * 5); // 4 HEX characters per 16 bit value + separator
  size_t zeroCount = 0;

  for (size_t i = 0; i < _bitmap.size(); ++i) {
    if (_bitmap[i] == 0) {
      ++zeroCount;
    } else if (zeroCount > 0) {
      result   += '(';
      result   += zeroCount;
      result   += ')';
      result   += separator;
      zeroCount = 0;
    }

    if (zeroCount == 0) {
      result += String(_bitmap[i], HEX);
      result += separator;
    }
  }
  return result;
}

String CreateBuildDescription(char separator) {
  String result;

  {
    result += 'T';
    xPluginEnumerator  cplugins;
    const unsigned int size = ProtocolIndex_to_CPlugin_id.size();
    cplugins.setSize(size);

    for (size_t i = 0; i < size; ++i) {
      cplugins.add(ProtocolIndex_to_CPlugin_id[i]);
    }
    result += cplugins.getString(separator);
  }
  {
    result += 'P';
    xPluginEnumerator  plugins;
    const unsigned int size = DeviceIndex_to_Plugin_id.size();
    plugins.setSize(size);

    for (size_t i = 0; i < size; ++i) {
      plugins.add(DeviceIndex_to_Plugin_id[i]);
    }
    result += plugins.getString(separator);
  }
  {
    // FIXME TD-er: Right now we don't have a notifierindex to ID vector

    /*
       result += 'N';
       xPluginEnumerator plugins;
       const unsigned int size = DeviceIndex_to_Plugin_id.size();
       plugins.setSize(size);
       for (size_t i = 0; i < size; ++i) {
        plugins.add(DeviceIndex_to_Plugin_id[i]);
       }
       result += plugins.getString(separator);
     */
  }
  return result;
}
