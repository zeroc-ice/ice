- A `ThreadPriority` property set to an unrecognized value now throws `PropertyException` instead of being
  silently ignored and falling back to the normal priority. This applies to `Ice.ThreadPriority` and to any
  thread pool's `ThreadPriority` property, such as `Ice.ThreadPool.Server.ThreadPriority` or an object adapter's
  `<adapter>.ThreadPool.ThreadPriority`. Integer values outside the allowed range (1-10) are also rejected.
- A thread pool's `ThreadPriority` property (such as `Ice.ThreadPool.Server.ThreadPriority` or an object
  adapter's `<adapter>.ThreadPool.ThreadPriority`) now also accepts the priority constant names `MIN_PRIORITY`,
  `NORM_PRIORITY`, and `MAX_PRIORITY`, like `Ice.ThreadPriority` already did. Previously these properties accepted
  only integer values.
