- Fixed `ImplicitContext.get`, `put`, and `remove` returning `undefined` instead of the empty string for a key that is
  not set. An `undefined` key is now treated as the empty-string key, like a `null` key.
