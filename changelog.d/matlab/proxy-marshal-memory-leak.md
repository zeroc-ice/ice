- Fixed a memory leak that occurred each time a proxy was marshaled, unmarshaled, or had its
  encoding version set with `ice_encodingVersion`. Each of these operations leaked a small amount
  of memory that accumulated over the lifetime of a MATLAB session.
