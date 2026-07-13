- Fixed the servant lookup for incoming requests: when the target identity is registered with a different facet, the
  object adapter now dispatches the request to the default servant if one is registered, as documented, instead of
  rejecting the request with `FacetNotExistException`.
