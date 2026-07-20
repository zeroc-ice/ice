- Fixed a bug on Windows where a plug-in or IceBox service configured with an unquoted UNC path (such as
  `\\server\share\plugin.jar`) was loaded from the wrong location: the path was resolved relative to the current
  directory instead of being recognized as absolute.
