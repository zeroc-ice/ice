To run the demo:

Start the IceStorm service:

$ icebox --Ice.Config=config.icebox

This configuration assumes there is a subdirectory named db in the
current working directory.

In a separate window:

$ subscriber

In another window:

$ publisher

While the publisher continues to run, "tick" messages should be
displayed in the subscriber window.

Both the subscriber and publisher take an optional topic name as a
final argument. The default value for this topic is "time".

Through the use of command-line options both the subscriber and
publisher can use different QoS for sending and receiving messages.

For the subscriber:

subscriber.exe --oneway

  The subscriber receives events as oneway messages. This is the
  default.

subscriber.exe --datagram

  The subscriber receives events as datagrams.

subscriber.exe --twoway

  The subscriber receives events as twoway messages.

subscriber.exe --ordered

  The subscriber receives events as twoway messages with guaranteed
  ordering.

subscriber.exe --batch

  This is an additional flag that forwards datagram and oneway events
  to the subscriber in batches.

For the publisher:

publisher.exe --oneway

  The publisher sends events as oneway messages. This is the default.

publisher.exe --datagram

  The publisher sends events as datagrams.

publisher.exe --twoway

  The publisher sends events as twoway messages.
