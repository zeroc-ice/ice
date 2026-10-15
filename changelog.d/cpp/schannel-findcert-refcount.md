- Fixed a reference-counting bug in the Schannel-based IceSSL transport (Windows): the certificates selected with
  `IceSSL.FindCert` were released by the certificate search and released again when the communicator was destroyed,
  which could corrupt memory.
