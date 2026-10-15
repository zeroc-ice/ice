- Fixed `Ice.initialize` to keep the reference counts of the `InitializationData` objects intact when it rejects an
  invalid `threadHook`, `threadStart`, `threadStop`, `dispatcher` or `batchRequestInterceptor`. It previously released
  references it did not own, which could later crash the interpreter.
