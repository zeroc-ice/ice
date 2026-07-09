- Fixed a bug in DataStorm where a sample discarded by the reader's discard policy left later partial updates on
  its key without a base value to resolve against.
- A DataStorm partial update without a base value is now dropped with a trace, instead of being applied to a
  default-constructed value — which typically crashed applications using class-typed values.
- A DataStorm reader-side updater or decoder that throws now drops only that sample and logs a warning; previously
  the exception could discard an entire batch of initialization samples.
