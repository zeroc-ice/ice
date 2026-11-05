- `Ice.Properties()`, `Ice.createProperties()`, `Ice.Communicator()`, and `Ice.initialize()` called without an argument
  vector no longer load the configuration files named by the `ICE_CONFIG` environment variable, as in the other
  language mappings. In addition, `Ice.Communicator` and `Ice.initialize` called with `--Ice.Config` now load only the
  files named by this option, and no longer also load the files named by `ICE_CONFIG`.
