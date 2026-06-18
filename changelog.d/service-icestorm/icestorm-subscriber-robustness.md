- Fixed an IceStorm bug where subscribing to a topic with an invalid (non-numeric) `retryCount` QoS value left that
  subscriber unable to be subscribed to the topic until IceStorm was restarted. The value is now rejected with
  `BadQoS`.
