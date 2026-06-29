- Fixed a memory leak in DataStorm where a session was not reclaimed when it was removed (or when its node was
  destroyed) while a retry timer was still pending for it: the timer held a strong reference back to the session
  that was never cleared. The leaked session also kept the node and instance it referenced alive. The most common
  trigger is a session to an endpoint-less peer that disconnects and does not reconnect.
