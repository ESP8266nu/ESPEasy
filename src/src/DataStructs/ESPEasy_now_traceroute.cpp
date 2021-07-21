#include "../DataStructs/ESPEasy_now_traceroute.h"

#include <list>

ESPEasy_now_traceroute_struct::ESPEasy_now_traceroute_struct(uint8_t size) {
  unit_vector.resize(size);
}

void ESPEasy_now_traceroute_struct::clear()
{
  unit_vector.clear();
}

uint8_t ESPEasy_now_traceroute_struct::getUnit(uint8_t distance, uint8_t& successRate) const
{
  if (unit_vector.size() < static_cast<size_t>((distance + 1) * 2)) {
    successRate = 0;
    return 0;
  }
  successRate = unit_vector[(distance * 2) + 1];
  return unit_vector[distance * 2];
}

void ESPEasy_now_traceroute_struct::addUnit(uint8_t unit)
{
  // Only add the unit if it isn't already part of the traceroute.
  const size_t index = unit_vector.size();
  for (size_t i = 0; i < index; i+=2) {
    if (unit_vector[i] == unit) {
      return;
    }
  }

  unit_vector.resize(index + 2);
  unit_vector[index]     = unit;
  // Set default success rate to average
  unit_vector[index + 1] = 127;
}

uint8_t ESPEasy_now_traceroute_struct::getDistance() const
{
  if (unit_vector.size() == 0) { return 255; }
  return (unit_vector.size() / 2) - 1;
}

const uint8_t * ESPEasy_now_traceroute_struct::getData(uint8_t& size) const
{
  size = static_cast<uint8_t>(std::min(unit_vector.size(), 255u));
  return &(unit_vector[0]);
}

uint8_t * ESPEasy_now_traceroute_struct::get()
{
  if (unit_vector.size() == 0) {
    return nullptr;
  }
  return &(unit_vector[0]);
}

void ESPEasy_now_traceroute_struct::setSuccessRate_last_node(uint8_t unit, uint8_t successRate)
{
  int index = unit_vector.size() - 2;
  int attempt = 0;

  while (index >= 0 && attempt < 2) {
    if (unit_vector[index] == unit) {
      unit_vector[index + 1] = successRate;
      return;
    }
    index -= 2;
    ++attempt;
  }
}

bool ESPEasy_now_traceroute_struct::operator<(const ESPEasy_now_traceroute_struct& other) const
{
  const int this_success = computeSuccessRate();
  const int other_success = other.computeSuccessRate();
  if (getDistance() == other.getDistance()) {
    // Same distance, just pick the highest success rate
    return this_success > other_success;
  }

  // If success rate of a route differs > 10 points, prefer the one with highest success rate
  if (this_success > (other_success + 10)) return true;
  if (other_success > (this_success + 10)) return false;

  // Both have somewhat equal success rate, pick one with shortest distance.
  return getDistance() < other.getDistance();
}

bool ESPEasy_now_traceroute_struct::sameRoute(const ESPEasy_now_traceroute_struct& other) const
{
  const uint8_t max_distance = getDistance();
  if (max_distance != other.getDistance()) {
    return false;
  }
  for (uint8_t distance = 0; distance <= max_distance; ++distance) {
    uint8_t success_rate = 0;
    if (getUnit(distance, success_rate) != other.getUnit(distance, success_rate)) {
      return false;
    }
  }
  return true;
}

int ESPEasy_now_traceroute_struct::computeSuccessRate() const
{
  const uint8_t max_distance = getDistance();
  if (max_distance == 255) {
    // No values stored, so huge penalty
    return 0;
  }
  if (max_distance == 0) {
    // End point, so assume best success rate
    return 255;
  }

  int res = 0;
  for (uint8_t distance = 0; distance <= max_distance; ++distance) {
    uint8_t successRate = 0;
    getUnit(distance, successRate);

    if (successRate < 50 && distance < max_distance) {
      return 0;
    }

    res += successRate;
  }
  if (max_distance > 1) {
    res /= max_distance;
  }
  /*
  res -= 10 * max_distance;
  if (res < 50) res = 50;
  */
  return res;
}

bool ESPEasy_now_traceroute_struct::unitInTraceRoute(uint8_t unit) const
{
  const uint8_t max_distance = getDistance();
  for (uint8_t distance = 0; distance <= max_distance; ++distance) {
    uint8_t success_rate = 0;
    if (getUnit(distance, success_rate) == unit) {
      return true;
    }
  }
  return false;
}

String ESPEasy_now_traceroute_struct::toString() const
{
  const uint8_t max_distance = getDistance();

  if (max_distance == 255) {
      return String();
  }
  String res;
  res.reserve(4*unit_vector.size());
  for (uint8_t distance = 0; distance <= max_distance; ++distance) {
    uint8_t success_rate = 0;
    res += getUnit(distance, success_rate);
    res += '/';
    res += success_rate;
    res += ' ';
  }
  return res;
}