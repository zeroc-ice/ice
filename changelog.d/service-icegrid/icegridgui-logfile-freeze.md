- Fixed the IceGrid GUI freezing when a log file window was closed or stopped while a read against an
  unresponsive node was still in progress. Stopping or closing a log window no longer blocks the UI thread
  waiting for the in-progress read to return.
