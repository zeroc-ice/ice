- Fixed a bug in DataStorm where, when two or more readers (or writers) reached an endpoint-less peer through the
  same relay node, the relay could fail to notify one of them when the peer disconnected, leaving it blocked
  forever in `waitForNoWriters` (or `waitForNoReaders`). The relay keyed the relayed sessions by an identity that
  is only unique within a node, so sessions from different nodes overwrote each other.
