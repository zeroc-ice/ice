- Fixed a memory leak in the Schannel-based IceSSL transport (Windows) where each failed TLS handshake leaked the
  security-token and alert buffers allocated by Schannel. On a server, a peer repeatedly failing handshakes could leak
  memory over time.
