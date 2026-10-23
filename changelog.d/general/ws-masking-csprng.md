- The C# and Java WebSocket transports now generate the frame masking keys and the `Sec-WebSocket-Key` handshake value
  with a cryptographically secure random number generator, as required by RFC 6455. C++ already did so.
