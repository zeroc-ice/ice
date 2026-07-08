- Fixed the Glacier2 address filters, `Glacier2.Filter.Address.Accept` and `Glacier2.Filter.Address.Reject`: a rule
  that did not end with a wildcard matched hosts that it does not describe. A rule without any wildcard, such as
  `api.example.com`, matched every host name ending with it, such as `notapi.example.com`, and a rule ending with a
  numeric range, such as `192.168.[0-255]`, matched every host name starting with it. An address rule now matches the
  host name in full. Review your address filters: a rule that relied on the previous behavior must now end with `*`.
