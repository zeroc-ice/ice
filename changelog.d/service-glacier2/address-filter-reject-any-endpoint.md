- Fixed the Glacier2 address filter `Glacier2.Filter.Address.Reject`: a reject rule matched a proxy only when every
  endpoint of the proxy matched the rule. A reject rule now matches—and the proxy is rejected—as soon as any endpoint
  of the proxy matches the rule.
