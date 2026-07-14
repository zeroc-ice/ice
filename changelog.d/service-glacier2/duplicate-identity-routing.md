- Fixed the routing of two different proxies with the same identity through a Glacier2 router: the router kept one
  of them arbitrarily and could route requests to the wrong proxy. It now rejects the conflicting proxy with an
  error.
