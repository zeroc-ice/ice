- The Glacier2 router now supports `createSessionFromSecureConnection` calls from clients connecting over secure
  WebSocket (wss), and sets the `_con.peerCert` context entry for these connections when
  `Glacier2.AddConnectionContext` is enabled.
