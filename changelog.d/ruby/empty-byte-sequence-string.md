- Fixed Ice for Ruby to correctly marshal an empty `sequence<byte>` supplied as a string: it now writes a single size
  byte instead of a raw 4-byte integer, which previously de-synchronized the rest of the message.
