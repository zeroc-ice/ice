- Fixed a slow memory accumulation in the IceGrid registry: a session destroyed by the client was retained by the
  registry until the connection used to create this session closed. This affects only registries with no node (a
  common setup when using dynamic registration) and no replica, with sessions created and destroyed through Glacier2,
  as Glacier2 maintains a long-lived connection to the registry.
