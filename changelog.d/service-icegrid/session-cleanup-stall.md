- Fixed a registry-wide stall in IceGrid. Previously, when a client that allocated a server with the `session`
  activation mode disconnected, the registry could temporarily stop accepting new sessions.
