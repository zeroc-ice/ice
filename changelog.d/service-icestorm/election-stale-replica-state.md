- Fixed an election bug in replicated IceStorm deployments. After a restart of all the replicas, or after a
  sequence of replica outages, the elected coordinator could adopt the state of a stale replica, silently
  discarding previously created topics and subscriptions.
