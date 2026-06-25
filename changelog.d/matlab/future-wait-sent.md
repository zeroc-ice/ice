- Fixed `Ice.Future.wait('sent')`, which could block indefinitely or time out even after the request had been
  sent. The wait now completes as soon as the invocation reaches or passes the requested state.
