- Fixed a bug in DataStorm where a reader connected to an any-key (filtered) writer could receive duplicate
  samples after a session reconnect: the writer re-sent its entire retained history instead of resuming after
  the samples the reader had already received.
