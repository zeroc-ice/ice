- `null` is now treated as equivalent to `Ice\None` for optional values, meaning they now write nothing on the wire.
  Previously: a null proxy was marshaled as set-but-null; a null string, sequence, or dictionary was marshaled as set-but-empty; and null was rejected with an `InvalidArgumentException` for other types.
