- Improved address filtering in the Glacier2 router. The router now normalizes the host of each endpoint before
  matching it against the `Glacier2.Filter.Address.Accept` and `Glacier2.Filter.Address.Reject` rules, so that a
  rule matches every equivalent spelling of the host it specifies. This normalization removes the trailing dot of
  a fully-qualified DNS name and converts IPv4 address literals to canonical dotted-quad form (for example, `0x7f.1`
  becomes `127.0.0.1`).
