- Fixed the `ice_getConnectionAsync` proxy method. Retrieving the result of the returned future failed on every
  successful call because the `Ice.Connection` was constructed without its communicator.
