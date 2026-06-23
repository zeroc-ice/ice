- Fixed a crash in `icestormadmin`: running the `replica` command against a non-replicated IceStorm instance
  dereferenced an empty node proxy. The command now reports that the instance is not replicated instead of crashing.
