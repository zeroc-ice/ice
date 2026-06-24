- Fixed `Ice::Service` in `--daemon` mode reporting success when the daemon exited during startup before
  signaling readiness (for example after being killed by a signal or aborting in a plug-in). The parent process
  now exits with a non-zero status so service supervisors don't conclude the service started.
