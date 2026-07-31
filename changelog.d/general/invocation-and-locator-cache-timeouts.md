- The invocation timeout is now normalized consistently in all language mappings and at all configuration surfaces
  (`ice_invocationTimeout`, the `InvocationTimeout` proxy property, and `Ice.Default.InvocationTimeout`): zero or
  any negative value means infinite and is normalized to -1, and a positive duration is rounded up to the next
  whole millisecond. Previously, C# kept a fractional timeout and `proxyToProperty` emitted it as-is
  (`InvocationTimeout=30.5`) — a value that `propertyToProxy` rejects.
- The locator cache timeout is now normalized consistently in all language mappings and at all configuration
  surfaces (`ice_locatorCacheTimeout`, the `LocatorCacheTimeout` proxy property, and
  `Ice.Default.LocatorCacheTimeout`): any negative value means infinite and is normalized to -1, 0 still means no
  caching, and a positive duration is rounded up to the next whole second. Previously, C# kept a fractional timeout
  and `proxyToProperty` emitted it as-is (`LocatorCacheTimeout=30.5`) — a value that `propertyToProxy` rejects.
