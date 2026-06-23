- Fixed a bug in DataStorm where destroying a topic while a peer was still attached to one of its elements did not
  detach the listeners, leaking them and (in debug builds) tripping an assertion.
