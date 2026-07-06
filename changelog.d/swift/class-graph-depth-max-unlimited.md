- Fixed communicator initialization to treat `Ice.ClassGraphDepthMax` values less than 1 as unlimited, matching the
  other language mappings. Previously, setting this property to 0 aborted the program with a `fatalError`.
