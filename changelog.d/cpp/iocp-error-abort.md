- On Windows, the Ice runtime now logs the error and calls `abort` when its I/O completion port fails with an internal
  error. Previously, the thread pool logged the error, waited five seconds, and then used an uninitialized event handler,
  which typically crashed the process. There is no clean way to recover from such an error.
