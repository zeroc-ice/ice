- Fixed a data race in batch request queuing that could corrupt batch-message framing (or crash) when one thread
  flushed batch requests on a connection, proxy, or communicator while another thread was making batch invocations on
  the same batch queue.
