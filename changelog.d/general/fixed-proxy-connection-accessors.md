- Fixed `ice_getConnection`, `ice_getConnectionAsync` and `ice_getCachedConnection` on a fixed proxy: these methods
  now always return the connection the proxy is bound to. Previously, `ice_getConnection` could throw when this
  connection was closed, and `ice_getCachedConnection` returned null until the proxy's first invocation.
