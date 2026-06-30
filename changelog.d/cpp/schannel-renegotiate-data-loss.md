- Fixed silent application-data loss in the Schannel-based IceSSL transport (Windows) when a TLS renegotiation (such as
  a TLS 1.3 KeyUpdate or NewSessionTicket) is received in the same read as already-decrypted application data. The
  plaintext extracted before the renegotiation request was dropped, corrupting the Ice protocol stream.
