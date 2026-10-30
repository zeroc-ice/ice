- Fixed `Communicator::flushBatchRequests` in C++, in the language mappings built on the C++ runtime, and in C#. When a
  connection was closed while the communicator was flushing its batch requests, the call could fail with an internal,
  non-Ice exception, and the batch requests queued on the remaining connections were not flushed.
