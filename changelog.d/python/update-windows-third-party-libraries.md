- Updated the bzip2 sources bundled with the Windows wheels from 1.0.6 to 1.0.8. The bzip2 update fixes an
  out-of-bounds write when decompressing a malformed compressed message, which a peer could trigger over the wire.
