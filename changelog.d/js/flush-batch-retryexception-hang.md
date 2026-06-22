- Fixed `Connection.flushBatchRequests` and `Communicator.flushBatchRequests` to report a proper Ice local exception
  when a connection is closed while its batch requests are being flushed, instead of leaking an internal exception.
