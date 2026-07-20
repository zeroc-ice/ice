- Fixed a bug in `icegridadmin`: after a `log --follow` command failed - for example because the target server was
  not reachable - all subsequent `log --follow` commands in the same session failed as well.
