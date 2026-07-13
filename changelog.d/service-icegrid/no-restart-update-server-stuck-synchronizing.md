- Fixed an IceGrid registry issue: after an application update that did not restart a running server (no-restart
  update), restarting the registry could leave this server stuck in a synchronizing state, causing administrative
  operations on this server to hang until the server was stopped.
