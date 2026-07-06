- Fixed a build failure of the OpenSSL-based IceSSL transport against OpenSSL 4.0, which makes `ASN1_STRING`
  opaque. Subject Alternative Name parsing now uses the `ASN1_STRING_get0_data`, `ASN1_STRING_length`, and
  `ASN1_STRING_type` accessors instead of direct struct member access.
