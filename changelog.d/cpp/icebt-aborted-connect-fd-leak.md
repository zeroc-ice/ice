- Fixed a socket leak in the IceBT transport: when an outgoing Bluetooth connection completed after the connection
  attempt was aborted, for example after a connect timeout, the socket of the new connection was never closed.
