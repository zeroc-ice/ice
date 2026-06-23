- Fixed a WebSocket bug where Ice, acting as a server, echoed a received ping's payload in the pong
  without unmasking it (and, as a client, sent the pong payload unmasked). A non-Ice WebSocket peer
  that sends payload-bearing pings and validates the echoed pong could therefore drop the connection
  (RFC 6455 §5.5.3). The common cases — browser/JS clients and empty keepalive pings — were unaffected.
