- `null` is now treated as equivalent to 'unset' for optional values, meaning they now write nothing on the wire.
  Previously: a null proxy or `java:serializable` object was encoded as set-but-null; a null string, sequence, or
  dictionary was encoded as set-but-empty; and a null struct or enum threw a `NullPointerException`.
