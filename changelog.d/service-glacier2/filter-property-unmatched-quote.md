- The Glacier2 router now refuses to start when a session filter property contains an unmatched quote, or when
  `Glacier2.Filter.Category.AcceptUser` is set to a value other than 0, 1, or 2. Previously, such values were
  silently accepted and could produce filters that did not match the intended configuration.
