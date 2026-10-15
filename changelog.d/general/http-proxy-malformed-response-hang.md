- Fixed the handling of an invalid response from an HTTP proxy (configured with `Ice.HTTPProxyHost`). Connection
  establishment now fails promptly with a `ProtocolException`, instead of hanging until the connection times out.
