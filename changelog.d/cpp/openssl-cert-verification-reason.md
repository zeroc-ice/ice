- Fixed certificate verification error reporting in the OpenSSL-based IceSSL transport. A rejected peer
  certificate now reports the specific reason (such as "certificate has expired") instead of a generic
  "rejected by the certificate validation callback" message.
