#include "../DataStructs/MessageRouteInfo.h"


MessageRouteInfo_t::MessageRouteInfo_t(const uint8_t* serializedData, size_t size) {
  deserialize(serializedData, size);
}

MessageRouteInfo_t::MessageRouteInfo_t(const uint8_t_vector& serializedData) {
  if (serializedData.size() > 0) {
    deserialize(&(serializedData[0]), serializedData.size());
  }
}

bool MessageRouteInfo_t::deserialize(const uint8_t_vector& serializedData) {
  if (serializedData.size() > 0) {
    return deserialize(&(serializedData[0]), serializedData.size());
  }
  return false;
}

bool MessageRouteInfo_t::deserialize(const uint8_t* serializedData, size_t size) {
  if (size >= 4 && serializedData != nullptr) {
    size_t index = 0;
    unit = serializedData[index++];
    count = serializedData[index++];
    dest_unit = serializedData[index++];
    const size_t traceSize = serializedData[index++];
    if (size >= (traceSize + 4)) {
      trace.resize(traceSize);
      for (size_t i = 0; i < traceSize; ++i) {
        trace[i] = serializedData[index++];
      }
      return true;
    }
  }
  return false;
}

MessageRouteInfo_t::uint8_t_vector MessageRouteInfo_t::serialize() const {
  uint8_t_vector res;
  res.resize(getSerializedSize());

  size_t index = 0;
  res[index++] = unit;
  res[index++] = count;
  res[index++] = dest_unit;
  res[index++] = static_cast<uint8_t>(std::min(trace.size(), 255u));
  for (size_t i = 0; i < trace.size(); ++i) {
    res[index++] = trace[i];
  }
  return res;  
}

size_t MessageRouteInfo_t::getSerializedSize() const {
  return 4 + trace.size();
}

bool MessageRouteInfo_t::appendUnit(uint8_t unitnr) {
  if (unitnr == 0 || unitnr == 255) {
    return false;
  }
  if (unit == 0) {
    unit = unitnr;
  } else {
    // First check we're not adding the same unitnr twice.
    auto it = trace.rbegin();
    if (it != trace.rend()) {
      if (*it == unitnr) {
        return true;
      }
    }

    trace.push_back(unitnr);
  }
  return true;
}

bool MessageRouteInfo_t::traceHasUnit(uint8_t unitnr) const {
  return countUnitInTrace(unitnr) != 0;
}

size_t MessageRouteInfo_t::countUnitInTrace(uint8_t unitnr) const {
  size_t res = 0;
  if (unitnr == 0 || unitnr == 255) {
    return res;
  }
  for (auto it = trace.begin(); it != trace.end(); ++it) {
    if (*it == unitnr) {
      ++res;
    }
  }
  if (unitnr == unit) {
    ++res;
  }
  return res;
}

uint8_t MessageRouteInfo_t::getLastUnitNotMatching(uint8_t unitnr) const {
  for (auto it = trace.rbegin(); it != trace.rend(); ++it) {
    if (*it != unitnr) {
      return *it;
    }
  }
  // Try the source unit
  if (unitnr != unit) return unit;
  return 0;
}

String MessageRouteInfo_t::toString() const {
  String res;
  res = F("src: ");
  res += unit;
  res += F(" dst: ");
  res += dest_unit;
  res += F(" path:");
  for (auto it = trace.begin(); it != trace.end(); ++it) {
    res += ' ';
    res += *it;
  }
  return res;
}


bool UnitMessageRouteInfo_map::isNew(const MessageRouteInfo_t *info) const {
  if (info == nullptr) { return true; }
  auto it = _map.find(info->unit);

  if (it != _map.end()) {
    return it->second.count != info->count;
  }
  return true;
}

void UnitMessageRouteInfo_map::add(const MessageRouteInfo_t *info) {
  if (info == nullptr) { return; }

  if ((info->unit != 0) && (info->unit != 255)) {
    _map[info->unit] = *info;
  }
}
