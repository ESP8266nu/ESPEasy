#ifndef DATASTRUCTS_SENDDATA_DUPLICATECHECKER_STRUCT_H
#define DATASTRUCTS_SENDDATA_DUPLICATECHECKER_STRUCT_H


#include "../DataStructs/SendData_DuplicateChecker_data.h"

#include "../Helpers/ESPEasyMutex.h"

#include <map>

// This duplicate checker keeps track of keys of previous messages for some time.
// If a message has been seen before, it will not be processed.
// New messages will be asked to peers.
// If a peer already has the message processed, then it has to reply within the timeout.
// Messages queried for by a peer will also be added to the historic list of seen messages.
class SendData_DuplicateChecker_struct {
public:

  static const uint32_t DUPLICATE_CHECKER_INVALID_KEY;

  // Add event to the queue and return key to ask around.
  uint32_t add(struct EventStruct *event,
               const String      & compare_key);

  // Check to see if we already know about some key.
  // If not yet seen, add it to our historic list since someone else is already processing it
  bool historicKey(uint32_t key);

  // Another node already reported to have handled the key
  void remove(uint32_t key);

  // Send out items in the queue not marked by other nodes as already seen.
  void loop();

private:

  void purge_old_historic();

  // Map of key + event
  std::map<uint32_t, SendData_DuplicateChecker_data>_queue;
  ESPEasy_Mutex _queue_mutex;

  // Map of key + timestamp last seen.
  std::map<uint32_t, uint32_t>_historic;
  ESPEasy_Mutex _historic_mutex;

};

#endif // DATASTRUCTS_SENDDATA_DUPLICATECHECKER_STRUCT_H
