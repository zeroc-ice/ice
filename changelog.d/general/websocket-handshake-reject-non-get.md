- Fixed the server-side WebSocket opening handshake to reject messages that are not `GET` requests, as required by
  RFC 6455. A peer could previously trip an assertion by sending a response-shaped handshake.
