- Fixed a crash in the iOS (SecureTransport) SSL transport: using `IceSSL.FindCert` to select a keychain
  certificate that has no label attribute could abort the process during communicator initialization. Ice now reports
  a clear error instead.
