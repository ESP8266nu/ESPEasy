#ifndef HELPERS_ESPEASY_BUILD_DESCRIPTION_H
#define HELPERS_ESPEASY_BUILD_DESCRIPTION_H

#include <Arduino.h>
#include <vector>

struct xPluginEnumerator {
public:

  xPluginEnumerator();

  void   setSize(unsigned int maxID);

  void   add(unsigned int ID);

  String getString(char separator) const;

private:

  std::vector<uint16_t>_bitmap;
};

String CreateBuildDescription(char separator);


#endif // HELPERS_ESPEASY_BUILD_DESCRIPTION_H
