- A `ThreadPriority` property set to an unrecognized value now throws `PropertyException` instead of being
  silently ignored and falling back to the normal priority. This applies to `Ice.ThreadPriority` and to any
  thread pool's `ThreadPriority` property, such as `Ice.ThreadPool.Server.ThreadPriority` or an object adapter's
  `<adapter>.ThreadPool.ThreadPriority`.
