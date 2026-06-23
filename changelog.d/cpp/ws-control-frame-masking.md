- Fixed a WebSocket bug where Ice did not unmask the payload of a received ping before echoing it in the pong, and
  did not mask the pong payload when sending as a client. A conforming WebSocket peer that sends payload-bearing pings
  and validates the returned pong (a common liveness probe) now receives the correct payload (RFC 6455 §5.5.3).
