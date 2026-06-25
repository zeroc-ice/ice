- Fixed a crash in the macOS (SecureTransport) SSL transport: configuring `IceSSL.CertFile` together with
  `IceSSL.KeyFile` using a certificate that has no Subject Key Identifier extension aborted the process during
  communicator initialization. Such certificates are now rejected with a `CertificateReadException`.
