- Fixed the IceGrid GUI freezing when a log file window was closed or stopped while a read against an
  unresponsive node was still in progress. The log reader is now joined off the UI thread, keeping the GUI
  responsive.
