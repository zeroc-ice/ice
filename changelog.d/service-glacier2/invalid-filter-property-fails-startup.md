- Fixed a bug where a malformed `Glacier2.Filter.Identity.Accept` or `Glacier2.Filter.Category.AcceptUser` property
  made every session creation hang instead of failing. The router now validates these properties at startup and
  refuses to start when one of them is invalid.
