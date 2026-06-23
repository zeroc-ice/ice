- Fixed the `icestormadmin` `replica` command when run against a non-replicated IceStorm instance. After reporting
  that the instance is not replicated, the command no longer reports a spurious `NullHandleException` error.
