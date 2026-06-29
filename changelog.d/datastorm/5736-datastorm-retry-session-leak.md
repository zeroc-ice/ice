- Fixed a memory leak in DataStorm where a node could fail to reclaim a session after losing its connection to a
  peer. The most common trigger is a peer with no public endpoint that disconnects and does not reconnect, so a
  long-running node leaked memory in proportion to the number of such peers seen over its lifetime.
