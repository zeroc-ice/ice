- Improved the validation of the Glacier2 address filter rules: a rule with an inverted numeric range (such as
  `[5-1]`) or with a stray `[` inside a group was accepted and then silently did not match the hosts it appears to
  describe.
