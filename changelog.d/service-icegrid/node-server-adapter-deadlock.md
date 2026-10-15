- Fixed a deadlock in the IceGrid node that could occur when a server was restarted while one of its object adapters
  was concurrently activated, deactivated, or queried. The node could hang, blocking all subsequent operations on
  that node.
