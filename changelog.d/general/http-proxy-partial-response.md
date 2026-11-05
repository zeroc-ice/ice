- Fixed the handling of a response from an HTTP proxy (configured with `Ice.HTTPProxyHost`) that is received in several
  pieces. Connection establishment could fail when the first read returned only part of the proxy's response.
