- On Apple platforms, a communicator configured with `IceSSL.VerifyPeer=1` now fails to initialize with an
  `InitializationException`. The SSL transport on these platforms cannot request an optional client certificate. Set
  `IceSSL.VerifyPeer` to 2 to require a client certificate, or to 0 to not request one; outgoing connections verify
  the server certificate the same way with 2 as with 1.
