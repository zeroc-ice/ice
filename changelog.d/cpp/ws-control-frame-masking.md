- Fixed a WebSocket bug where an Ice server echoed a received ping's payload in the pong without
  unmasking it, so a non-Ice WebSocket peer that sends payload-bearing pings and validates the echoed
  pong (e.g. an L7 load balancer or gateway health check) could drop the connection (RFC 6455 §5.5.3).
  The common cases — browser/JS clients and empty keepalive pings — were unaffected.
