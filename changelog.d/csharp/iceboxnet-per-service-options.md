- Fixed `iceboxnet` rejecting valid per-service command-line options (`--<service>.*`) with "unknown option" and
  failing to start: the option validation iterated the original arguments instead of the filtered list.
