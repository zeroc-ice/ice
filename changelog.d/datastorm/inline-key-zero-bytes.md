- Fixed the handling of a key that a custom `Encoder` encodes to an empty byte sequence. An any-key or filtered
  writer marshals such a key inline like any other, but a reader mistook it for a key sent by id: it discarded the
  sample, or delivered it without checking the key against the reader's own subscription.
