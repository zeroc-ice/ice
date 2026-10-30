- Fixed `Ice.Future.wait` with an infinite or very large timeout, which returned `false` immediately instead of
  waiting for the requested state. An infinite timeout now waits indefinitely, a `NaN` timeout is rejected, and the
  timed wait is no longer affected by changes to the system clock.
