- The Glacier2 router now fails to start when the proxy configured by `Glacier2.PermissionsVerifier`,
  `Glacier2.SessionManager`, `Glacier2.SSLPermissionsVerifier`, or `Glacier2.SSLSessionManager` cannot be parsed.
  Previously, the router logged a "unable to contact" warning and started without the corresponding verifier or
  session manager.
