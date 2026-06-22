- Fixed a hang in `Communicator::flushBatchRequests` and `flushBatchRequestsAsync`. When a connection was closed while
  the communicator was flushing its batch requests, the returned future or completion callback could remain pending
  forever.
