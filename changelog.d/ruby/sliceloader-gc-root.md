- Fixed Ice for Ruby to keep a custom `SliceLoader` alive for the communicator's lifetime. The wrapper previously
  registered the wrong address with the garbage collector, leaving a dangling root and allowing the loader to be
  collected while still in use.
