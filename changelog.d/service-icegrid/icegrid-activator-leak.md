- Fixed a file descriptor leak in the IceGrid node. When server activation failed because the host was out of
  file descriptors or processes, the node leaked the pipes it had already opened, worsening the shortage.
- Fixed a shutdown hang in the IceGrid node on Windows that could occur when the node failed to register the
  wait handler for a server it had just started.
