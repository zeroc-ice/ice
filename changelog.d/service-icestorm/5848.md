- Fixed an election bug in replicated IceStorm deployments. When a replication failure was detected while a
  replica was taking part in an election — for example during a master failover with updates in flight — the
  replica could stop serving requests until it was restarted.
