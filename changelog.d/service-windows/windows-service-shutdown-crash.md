- Fixed a crash on shutdown. When an Ice service running as a Windows service was stopped using an Ice admin tool —
  for example, an IceGrid node shut down with `icegridadmin` or the IceGrid GUI — instead of through the Windows
  Service Control Manager, the process terminated abnormally rather than stopping cleanly (and never reported
  `SERVICE_STOPPED`). This affected the IceGrid registry and node, the Glacier2 router, and IceBox.
