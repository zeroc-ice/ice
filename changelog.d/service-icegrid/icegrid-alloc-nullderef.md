- Fixed a crash in an IceGrid replica registry that could occur when an allocatable object moved from one application
  to another while the replica was disconnected from the master, and the application now holding this object was later
  updated or removed.
