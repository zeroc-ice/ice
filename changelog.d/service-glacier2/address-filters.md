- Fixed several bugs in the Glacier2 address filters, `Glacier2.Filter.Address.Accept` and
  `Glacier2.Filter.Address.Reject`:
  - An address rule now matches the host name in full. Previously, a rule without any wildcard, such as
    `api.example.com`, matched every host name ending with it, such as `notapi.example.com`, and a rule ending with a
    numeric range, such as `192.168.[0-255]`, matched every host name starting with it. Review your address filters
    if a rule relied on the previous behavior: prepend `*` to restore a suffix match (`*api.example.com`), and append
    `*` to restore a prefix match (`192.168.[0-255]*`).
  - A reject rule now rejects a proxy as soon as one of its endpoints matches the rule. Previously, a multi-endpoint
    proxy matched a reject rule only when all its endpoints matched it.
  - Host names now match regardless of case, and the trailing dot of a fully-qualified DNS name is ignored, in the
    endpoint host and in the rules alike: a rule written for `backend.example.com` matches the host
    `Backend.Example.COM.`.
  - When an address filter is configured, the router now rejects a proxy when any of its endpoints has a host that
    the rules cannot match reliably: an IPv4 address written in a non-canonical form, such as `0x7f000001`,
    `2130706433`, `0177.0.0.1` or `127.1`, or with a trailing dot; an empty host; a host containing a space or a
    control character; a host longer than 255 characters; or an endpoint that is not an IP endpoint, such as an
    opaque endpoint of a transport the router does not know. Configurations without address filters are not
    affected.
  - A rule with a wildcard directly followed by a numeric range, such as `*[0-255]`, could make the router loop
    forever while checking a proxy whose host contains a long run of digits.
