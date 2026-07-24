- Fixed the handling of partial updates for keys with no value. A key has no value when it was removed, or when no
  full value was written for it yet. Previously, such a partial update crashed applications using class-typed
  values; with other value types, it silently resurrected removed keys. Now a writer throws `std::logic_error`
  when publishing such an update, and a reader discards incoming ones.
- Fixed a bug where calling `getValue()` on a `Remove` sample could return indeterminate data instead of the
  documented default value.
