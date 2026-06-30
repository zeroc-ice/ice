- Fixed Ice on the OpenSSL transport to use `IceSSL.Password` when loading an encrypted PEM private key. Previously the
  password was ignored, so OpenSSL prompted for it on the terminal or failed to load the key.
