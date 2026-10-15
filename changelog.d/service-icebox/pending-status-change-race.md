- Fixed a race condition in the IceBox service manager: when several `startService` or `stopService` calls were in
  progress, shutting down the IceBox server waited only until the first of them completed, and could then stop the
  services while the other calls were still starting or stopping services.
