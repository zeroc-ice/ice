- Fixed a data race in batch request queuing that could corrupt batch-message framing (or crash) when one thread flushed
  batch requests on a connection, proxy, or communicator while another thread was making batch invocations on the same
  batch queue. This race could only occur in the multi-threaded mappings: C++, C#, Java, Objective-C, and Python
  (Objective-C and Python use the C++ runtime).
