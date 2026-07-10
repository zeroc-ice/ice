- Fixed transient IceStorm (`IceStorm.Transient=1`) to reject `subscribeAndGetPublisher` and `link` calls on a
  destroyed topic with `ObjectNotExistException`, matching the persistent implementation. Previously such a call
  succeeded on a topic that could no longer deliver events and could permanently block re-subscribing that
  subscriber.
