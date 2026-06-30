- Fixed `IceSSL.Password` not being used when loading an encrypted PEM private key in the OpenSSL-based IceSSL
  transport. Previously OpenSSL prompted for the password on the terminal or failed to load the key.
