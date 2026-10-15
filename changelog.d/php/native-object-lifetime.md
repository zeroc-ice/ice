- Fixed crashes in Ice for PHP when using a clone of a connection, endpoint, or endpoint info object, and when calling
  `Ice\find` for a registered communicator after the PHP object previously returned for it in the same request was
  destroyed. Connection, endpoint, and endpoint info objects are no longer cloneable: `clone` now raises an error.
