- Fixed a bug in the Glacier2 address filters, `Glacier2.Filter.Address.Accept` and
  `Glacier2.Filter.Address.Reject`: a rule that contains a numeric range preceded by a wildcard, such as
  `*.[0-255]`, could send the router into an infinite loop while checking a proxy.
