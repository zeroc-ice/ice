- Fixed a bug in DataStorm where a partial update for a key with no current value—no full value was written for the
  key, or the key was removed—was resolved against an invalid base value: applications using class-typed values
  typically crashed, and with other value types the update was silently applied to a default-constructed value,
  resurrecting removed keys. A remove now clears the key's value, and a partial update for a key with no value is
  discarded, by both writers and readers.
- Fixed a bug in DataStorm where a remove sample's value was indeterminate for scalar value types: `getValue()` on
  the remove sample returned arbitrary data instead of a default-constructed value.
