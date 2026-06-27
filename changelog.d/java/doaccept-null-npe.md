- Fixed Ice for Java to no longer log a spurious `NullPointerException` (at error level, with a stack trace) when a
  client resets a pending connection during `accept()`. The accept-path race is now handled like any other accept
  failure: silent by default, or a one-line warning when `Ice.Warn.Connections` is set.
