- Fixed a bug in IceDiscovery where the replica-group endpoint aggregation window was about 10 times longer than
  intended, due to an incorrect nanosecond-to-millisecond conversion. As a result, resolving an indirect proxy bound to
  a replica group took noticeably longer than the configured `IceDiscovery.LatencyMultiplier` implies.
