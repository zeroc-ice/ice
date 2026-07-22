- Fixed a bug in DataStorm where a partial update for a key with no current value was resolved against an invalid
  base value: applications using class-typed values typically crashed, and with other value types the update was
  silently applied to a default-constructed value, resurrecting removed keys. A key has no current value when no full
  value was written for it yet, or when it was removed. A remove now clears the key's value; publishing a partial
  update for a key with no value now throws `std::logic_error` on the writer and is discarded on the reader.
- Fixed a bug in DataStorm where a remove sample's value was indeterminate for scalar value types: `getValue()` on
  the remove sample returned arbitrary data instead of a default-constructed value.
