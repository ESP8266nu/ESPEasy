#ifndef DATASTRUCTS_ESPEASY_NOW_TRACEROUTE_H
#define DATASTRUCTS_ESPEASY_NOW_TRACEROUTE_H

#include "../../ESPEasy_common.h"

#include <vector>
#include <map>

/*********************************************************************************************\
* ESPEasy-NOW Trace route
\*********************************************************************************************/
struct ESPEasy_now_traceroute_struct
{
  ESPEasy_now_traceroute_struct() = default;

  ESPEasy_now_traceroute_struct(uint8_t size);

  void clear();

  // Return the unit and RSSI given a distance.
  // @retval 0 when giving invalid distance.
  uint8_t getUnit(uint8_t distance,
                  uint8_t& successRate) const;

  // Append unit at the end (thus furthest distance)
  void           addUnit(uint8_t unit);

  uint8_t        getDistance() const;

  const uint8_t* getData(uint8_t& size) const;

  // Get pointer to the raw data.
  // Make sure the array is large enough to store the data.
  uint8_t* get();

  void     setSuccessRate_last_node(uint8_t unit, uint8_t successRate);

  // Return true when this tracerouteis more favorable
  bool     operator<(const ESPEasy_now_traceroute_struct& other) const;

  bool sameRoute(const ESPEasy_now_traceroute_struct& other) const;
  // For debugging purposes
  String toString() const;

  // Compute success rate
  int computeSuccessRate() const;

  bool unitInTraceRoute(uint8_t unit) const;

private:

  // Node with distance 0 at front, so index/2 equals distance.
  // index%2 == 0 is unit
  // index%2 == 1 is SuccessRate
  std::vector<uint8_t>unit_vector;
};

typedef std::map<uint8_t, ESPEasy_now_traceroute_struct> TraceRouteMap;

#endif // ifndef DATASTRUCTS_ESPEASY_NOW_TRACEROUTE_H
