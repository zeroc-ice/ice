- Fixed the TrustManager in C# and Java to properly handle the rare situation where Ice's RFC 2253 parser cannot parse a
  certificate's subject DN string produced by the platform's SSL stack. Previously, a parse failure could bypass
  `IceSSL.TrustOnly` reject rules.
