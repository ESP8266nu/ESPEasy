#ifndef DATASTRUCTS_MAC_ADDRESS_H
#define DATASTRUCTS_MAC_ADDRESS_H

#include <stdint.h>
#include <WString.h>

class __attribute__((__packed__)) MAC_address {
public:

  MAC_address();

  MAC_address(const uint8_t new_mac[6]);

  bool operator==(const MAC_address& other) const {
    return mac_addr_cmp(other.mac);
  }

  bool operator!=(const MAC_address& other) const {
    return !mac_addr_cmp(other.mac);
  }

  bool operator==(const uint8_t other[6]) const {
    return mac_addr_cmp(other);
  }

  bool operator!=(const uint8_t other[6]) const {
    return !mac_addr_cmp(other);
  }

  // Parse string with MAC address.
  // Returns false if the given string has no valid formatted mac address.
  bool   set(const char *string);

  void   set(const uint8_t other[6]);

  void   get(uint8_t mac_out[6]) const;

  bool   all_zero() const;

  bool   all_one() const;

  String toString() const;


  // An universally administered address (UAA) is uniquely assigned to a device by its manufacturer.
  // The first three octets (in transmission order) identify the organization that issued
  // the identifier and are known as the organizationally unique identifier (OUI)
  bool isUniversal() const {
    return (mac[0] & 2) == 0;
  }

  // A locally administered address (LAA) is assigned to a device by a network administrator, overriding the burned-in address.
  bool isLocal() const {
    return !isUniversal();
  }

  // Unicast frames are meant to be received by a single network device.
  // See: https://en.wikipedia.org/wiki/MAC_address#Unicast_vs._multicast
  bool isUnicast() const {
    return (mac[0] & 1) == 0;
  }

  // Multicast frames are meant to be received by multiple network devices
  // See: https://en.wikipedia.org/wiki/MAC_address#Unicast_vs._multicast
  bool isMulticast() const {
    return !isUnicast();
  }

  uint8_t mac[6] = { 0 };

private:

  bool mac_addr_cmp(const uint8_t other[6]) const;
};

#endif // DATASTRUCTS_MAC_ADDRESS_H
