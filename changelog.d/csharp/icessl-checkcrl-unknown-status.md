- With `IceSSL.CheckCRL=1`, a connection was rejected when the revocation status of the peer's own certificate could
  not be determined, for example because its OCSP responder or CRL distribution point was unreachable. The value `1`
  now accepts such a certificate again, as in Ice 3.7, and rejects it only when it is known to be revoked.
  `IceSSL.CheckCRL=2` still rejects a certificate whose revocation status cannot be determined.
