- Fixed a bug where a `DataStorm::Node` constructor failure, such as an invalid `DataStorm.Node.ConnectTo`
  endpoint, aborted the process instead of throwing an exception.
- Fixed a bug where an invalid `DataStorm.Topic.*` property value aborted the process when a topic reader or writer
  was first created. The topic default configurations are now parsed by the `Node` constructor, so an invalid value
  surfaces there as an exception. Additionally, the `DataStorm.Topic.DiscardPolicy` property now accepts `None` as
  an alias for `Never`, matching the `DiscardPolicy::None` enumerator.
- Fixed a bug where `Node::getSessionConnection` aborted the process when the given session identity was malformed,
  instead of returning null.
- Fixed a bug where calling `shutdown`, `isShutdown`, or `waitForShutdown` on a moved-from `Node` crashed.
