- Improved the Glacier2 address filters, `Glacier2.Filter.Address.Accept` and `Glacier2.Filter.Address.Reject`: the
  filters now normalize the host of each endpoint before matching it, so that all equivalent spellings of the same
  host match the same rules. The normalization removes the trailing dot of a fully-qualified DNS name and converts
  IPv4 address literals to canonical dotted-quad form. Write IPv4 rules in canonical form (`127.0.0.1`): a rule
  spelled in a non-canonical form no longer matches.
