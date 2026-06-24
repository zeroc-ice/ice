- Fixed a bug in DataStorm where destroying a topic before the readers and writers created from it, while a peer was
  still attached to one of them, leaked the listeners and (in debug builds) tripped an assertion. This is uncommon,
  since readers and writers are normally destroyed before their topic.
