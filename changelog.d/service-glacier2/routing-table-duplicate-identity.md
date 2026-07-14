- Fixed a bug in the Glacier2 routing table: registering a proxy with the same identity as an existing entry but a
  different address had an undefined result. The router now rejects such a request.
