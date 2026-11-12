- Fixed `Connection.flushBatchRequests` on a closed connection: the returned promise is now rejected with the exception
  that closed the connection, such as `ConnectionManuallyClosedException`, instead of an internal Ice exception.
