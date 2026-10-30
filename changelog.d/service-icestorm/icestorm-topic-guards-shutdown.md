- Fixed a hang that could prevent IceStorm from shutting down cleanly when a subscriber still had requests outstanding
  to an unreachable endpoint.
- Fixed a race in IceStorm where destroying a topic concurrently with a subscribe or link to the same topic could leave
  an orphaned record in the database, causing the destroyed topic to reappear the next time IceStorm was restarted.
