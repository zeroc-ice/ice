- Fixed a registry-wide stall in IceGrid. Previously, when a client that allocated a server with the `session`
  allocation mode suddenly disconnected, the registry could temporarily stop accepting all new sessions.
