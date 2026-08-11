- The `Ice.Communicator` constructor and `Ice.initialize` in Ice for Python now raise `ValueError` when the `initData`
  argument is combined with the `args` or `eventLoop` argument. These arguments are mutually exclusive; previously the
  extra argument was silently ignored, except for `Ice.Communicator(args, initData=initData)`, which replaced
  `initData` with an object derived from `args`.
