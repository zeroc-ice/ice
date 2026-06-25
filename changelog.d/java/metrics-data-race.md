- Fixed a data race in the Ice for Java metrics (IceMX) implementation. Reconfiguring the metrics views at
  runtime while metrics were being collected could corrupt the internal metrics maps or throw a
  `ConcurrentModificationException`.
