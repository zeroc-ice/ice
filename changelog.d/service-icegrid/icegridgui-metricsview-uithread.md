- Fixed a Swing threading violation in the IceGrid GUI: when refreshing a metrics view failed synchronously,
  the error dialog was created off the UI thread, which could corrupt the display or deadlock. The dialog is
  now shown on the UI thread.
