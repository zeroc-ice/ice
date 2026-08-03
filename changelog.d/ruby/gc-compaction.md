- Fixed a bug where GC heap compaction (`GC.compact` or `GC.auto_compact = true`) broke the Ice extension: after a
  compaction, the proxies, classes, exceptions, and enumerators returned by Ice invocations could be instances of
  unrelated classes, and the Ruby interpreter could crash.
