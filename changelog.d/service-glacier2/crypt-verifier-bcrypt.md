- Fixed the Glacier2 Crypt permissions verifier (`Glacier2.CryptPasswords`), which denied every login of a user whose
  password file entry is a bcrypt hash (`$2a$`, `$2b$` or `$2y$`). The verifier now accepts bcrypt hashes on Linux and
  FreeBSD when the system's crypt library supports them.
