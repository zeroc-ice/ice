- Fixed a bug where a failure to bind a UDP endpoint, for example when activating an object adapter whose UDP endpoint
  uses an address that is not available on the host, closed the socket twice. The second close could close an unrelated
  file descriptor opened concurrently by another thread.
