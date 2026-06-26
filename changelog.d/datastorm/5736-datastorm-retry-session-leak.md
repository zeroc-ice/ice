- Fixed a memory leak in DataStorm where removing a session whose peer had no public endpoint leaked the session
  (and, through it, the node and instance). The session's pending retry timer held a strong reference back to the
  session that was never cleared, so the session was never reclaimed. A long-running node leaked one such session
  for every endpoint-less peer that disconnected and did not reconnect.
