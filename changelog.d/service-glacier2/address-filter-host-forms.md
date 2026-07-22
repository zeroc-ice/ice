- When address filters are configured (`Glacier2.Filter.Address.Accept` or `Glacier2.Filter.Address.Reject`), the
  Glacier2 router now rejects a proxy when any of its endpoints has a host that writes an IPv4 address in a
  non-canonical form: for example, `0x7f000001`, `2130706433`, and `127.1` are all non-canonical forms of
  `127.0.0.1`. The filter rules match the host as a string, so previously a host in such a form matched neither
  the Accept nor the Reject rules written for the usual dotted-quad form.

- The Glacier2 address filters now ignore the trailing dot of a fully-qualified DNS name, in the endpoint host and
  in the Accept and Reject rules alike: a rule written for `backend.example.com` matches the host
  `backend.example.com.`, and vice versa. Previously, the host and the rule matched only when both spelled the
  trailing dot the same way.
