- The Schannel-based IceSSL transport (Windows) now rejects a PEM certificate file that contains a
  `-----BEGIN CERTIFICATE-----` marker without a matching `-----END CERTIFICATE-----` marker with a
  `PluginInitializationException`. Previously, the size computed for such a certificate was invalid, and loading the
  file could fail with a non-Ice exception.
