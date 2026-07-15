- Fixed a bug in the Glacier2 Crypt permissions verifier: a password file with more than one entry for the same user
  kept the first entry and silently ignored the others. A duplicate entry now fails startup, like a malformed entry.
