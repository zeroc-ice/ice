- Fixed the Schannel (Windows) SSL transport to reject mid-stream TLS renegotiation with a clear error instead of
  aborting the connection with a mislabeled `ConnectionLostException`.
