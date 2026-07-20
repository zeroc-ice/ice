- Fixed a bug in the Glacier2 address filters, `Glacier2.Filter.Address.Accept` and `Glacier2.Filter.Address.Reject`:
  the filters compared host names case-sensitively. Host names now match regardless of case, like DNS names.
