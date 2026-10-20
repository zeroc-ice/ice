- Fixed the Glacier2 address filters, `Glacier2.Filter.Address.Accept` and `Glacier2.Filter.Address.Reject`: a rule that
  did not end with a wildcard matched hosts that it does not describe. A rule without any wildcard, such as
  `api.example.com`, matched every host name ending with it, such as `notapi.example.com`, and a rule ending with a
  numeric range, such as `192.168.[0-255]`, matched every host name starting with it. An address rule now matches the
  host name in full. Review your address filters if a rule relied on the previous behavior: prepend `*` to restore a
  suffix match (`*api.example.com` matches `notapi.example.com` again), and append `*` to restore a prefix match
  (`192.168.[0-255]*` matches `192.168.5.5` again).

- Additional fixes and improvements to the address filters:
  - Host names now match regardless of case, like DNS names.
  - The filters now ignore the trailing dot of a fully-qualified DNS name, in the endpoint host and in the Accept and
    Reject rules alike: a rule written for `backend.example.com` matches the host `backend.example.com.`, and vice
    versa.
  - When address filters are configured, the router now rejects a proxy when any of its endpoints has a host that writes
    an IPv4 address in a non-canonical form: for example, `0x7f000001`, `2130706433`, and `127.1` are all non-canonical
    forms of `127.0.0.1`. Previously, such a host matched neither the Accept nor the Reject rules written for the usual
    dotted-quad form.
  - The `Glacier2.Filter.Address.Reject` filter now filters multi-endpoint proxies correctly.
  - A rule that contains a numeric range preceded by a wildcard, such as `*.[0-255]`, no longer sends the router into an
    infinite loop while checking a proxy.
