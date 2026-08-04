- Fixed a bug where receiving a Slice user exception leaked memory. A program that received many user exceptions
  became progressively slower and could eventually crash.
