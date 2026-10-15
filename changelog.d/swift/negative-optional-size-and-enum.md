- Fixed the unmarshaling of malformed data: a negative size for an optional value now throws `MarshalException`, and a
  negative encoding 1.0 enumerator value now throws `UnmarshalOutOfBoundsException`, instead of corrupting the stream
  position or crashing.
