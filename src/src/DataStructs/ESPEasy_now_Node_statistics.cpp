#include "../DataStructs/ESPEasy_now_Node_statistics.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Helpers/ESPEasy_time_calc.h"

#ifdef USES_ESPEASY_NOW

unsigned long ESPEasy_now_Node_statistics_t::getAge() const
{
  return timePassedSince(last_update_route[last_route_index]);
}

void ESPEasy_now_Node_statistics_t::addRoute(uint8_t unit, const ESPEasy_now_traceroute_struct& route)
{
  if (route.getDistance() == 255) {
    return;
  }

  if (last_update_route[last_route_index] != 0 && timePassedSince(last_update_route[last_route_index]) < 1000) {
    // Handling a burst of updates, only add those which have a higher success rate.
    if (routes[last_route_index] < route) {
      return;
    }
  }

  ++last_route_index;

  if (last_route_index >= ESPEASY_NOW_NODE_STATISTICS_NR_ROUTES) {
    last_route_index = 0;
  }
  routes[last_route_index] = route;
  routes[last_route_index].addUnit(unit);
  routes[last_route_index].setSuccessRate_last_node(unit, success_rate);
  last_update_route[last_route_index] = millis();
  last_update                         = millis();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F(ESPEASY_NOW_NAME);
    log += F(": addRoute: ");
    log += route.toString();
    addLog(LOG_LEVEL_INFO, log);
  }
}

void ESPEasy_now_Node_statistics_t::setDiscoveryRoute(uint8_t unit, const ESPEasy_now_traceroute_struct& route)
{
  discovery_route = route;
  discovery_route.addUnit(unit);
  discovery_route.setSuccessRate_last_node(unit, success_rate);
}

void ESPEasy_now_Node_statistics_t::updateSuccessRate(uint8_t unit, bool success)
{
  if (success) {
    if (timePassedSince(last_update) < 100) {
      // Apply some rate limiter.
      return;
      //if (success_rate > 100) { --success_rate; }
    } else if (success_rate < 255) { ++success_rate; }
    last_update = millis();
  } else {
    if (success_rate > 0) { --success_rate; }
  }

  for (unsigned int i = 0; i < ESPEASY_NOW_NODE_STATISTICS_NR_ROUTES; ++i) {
    if (last_update_route[i] != 0 && timePassedSince(last_update_route[i]) > 125000) {
      last_update_route[i] = 0;
      routes[i].clear();
    } else {
      routes[i].setSuccessRate_last_node(unit, success_rate);
    }
  }
  discovery_route.setSuccessRate_last_node(unit, success_rate);
}

uint8_t ESPEasy_now_Node_statistics_t::getNodeSuccessRate() const
{
  return success_rate;
}

ESPEasy_now_traceroute_struct& ESPEasy_now_Node_statistics_t::latestRoute()
{
  return routes[last_route_index];
}

const ESPEasy_now_traceroute_struct& ESPEasy_now_Node_statistics_t::latestRoute() const
{
  return routes[last_route_index];
}

const ESPEasy_now_traceroute_struct * ESPEasy_now_Node_statistics_t::bestRoute() const
{
  int bestIndex        = -1;
  int bestSuccessRate  = 0;
  uint8_t bestDistance = 255;

  for (int i = 0; i < ESPEASY_NOW_NODE_STATISTICS_NR_ROUTES; ++i) {
    const uint8_t distance    = routes[i].getDistance();
    const int     successRate = routes[i].computeSuccessRate();

    if (distance == bestDistance) {
      if (successRate > bestSuccessRate && distance < 255) {
        bestSuccessRate = successRate;
        bestIndex       = i;
      }
    } else if (distance < bestDistance) {
      bestIndex       = i;
      bestDistance    = distance;
      bestSuccessRate = successRate;
    }
  }

  if (bestIndex >= 0) {
    return &routes[bestIndex];
  }

  return nullptr;
}

const ESPEasy_now_traceroute_struct& ESPEasy_now_Node_statistics_t::discoveryRoute() const
{
  return discovery_route;
}

ESPEasy_Now_MQTT_queue_check_packet::QueueState ESPEasy_now_Node_statistics_t::getMQTTQueueState() const
{
  return mqtt_queue_state;
}

void ESPEasy_now_Node_statistics_t::setMQTTQueueState(ESPEasy_Now_MQTT_queue_check_packet::QueueState state)
{
  mqtt_queue_state = state;
}

#endif