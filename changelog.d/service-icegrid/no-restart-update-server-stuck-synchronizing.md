- Fixed an IceGrid issue: after an application update that did not restart a server (no-restart update) and that
  reached the node while this server was activating, restarting the registry could leave the server stuck in a
  synchronizing state. Administrative operations on this server failed, and updates of its application hung, until
  the server was stopped.
