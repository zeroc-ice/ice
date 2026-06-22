- Fixed a hang in `Communicator.flushBatchRequestsAsync`. When a connection was closed while the communicator was
  flushing its batch requests, the returned task never completed.
