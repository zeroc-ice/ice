- Fixed `Connection.flushBatchRequests` to reject with an Ice local exception when the connection is closed while its
  batch requests are being flushed, instead of rejecting with an internal exception. `Communicator.flushBatchRequests`
  no longer fails with this internal exception.
