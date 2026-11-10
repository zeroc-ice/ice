- Changed when Ice for MATLAB loads the configuration files named by the `ICE_CONFIG` environment variable.
  Applications that don't set `ICE_CONFIG` are not affected.
  - `Ice.Properties`, `Ice.createProperties`, `Ice.Communicator`, and `Ice.initialize` called without an argument
    vector no longer load these files, as in the other language mappings. To load them, pass an argument vector,
    even an empty one: for example, `Ice.Communicator(string.empty)`.
  - `Ice.Communicator` and `Ice.initialize` called with `--Ice.Config` now load only the files named by this option.
    Previously, they also loaded the files named by `ICE_CONFIG`.
