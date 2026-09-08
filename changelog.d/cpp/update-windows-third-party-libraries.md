- Updated the third-party libraries bundled with the Windows build: bzip2 from 1.0.6 to 1.0.8, Expat from 2.4.1 to
  2.8.4, and LMDB from 0.9.29 to 0.9.36. The bzip2 update fixes an out-of-bounds write when decompressing a
  malformed compressed message, which a peer could trigger over the wire.
