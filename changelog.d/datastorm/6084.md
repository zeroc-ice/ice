- Fixed a bug where a full value that a custom `Encoder` encodes to an empty byte sequence was decoded as
  value-less. Such a value could not serve as a base for a subsequent partial update, which was then silently
  discarded. The value is now decoded through its `Decoder` like any other full value, and the `Decoder` defines
  the meaning of empty input.
