Overview
--------

To run the demo:

Start the IceStorm service:

$ icebox --Ice.Config=config.icebox

This configuration assumes there is a subdirectory named db in the
current working directory.

In a separate window:

$ java -jar build/libs/subscriber.jar

In another window:

$ java -jar build/libs/publisher.jar

While the publisher continues to run, "tick" messages should be
displayed in the subscriber window.


Options
-------

Both the subscriber and publisher take an optional topic name as a
final argument. The default value for this topic is "time".

Through the use of command-line options both the subscriber and
publisher can use different QoS for sending and receiving messages.

For the subscriber:

java -jar build/libs/subscriber.jar --oneway

  The subscriber receives events as oneway messages. This is the
  default.

java -jar build/libs/subscriber.jar --datagram

  The subscriber receives events as datagrams.

java -jar build/libs/subscriber.jar --twoway

  The subscriber receives events as twoway messages.

java -jar build/libs/subscriber.jar --ordered

  The subscriber receives events as twoway messages with guaranteed
  ordering.

java -jar build/libs/subscriber.jar --batch

  This is an additional flag that forwards datagram and oneway events
  to the subscriber in batches.

java -jar build/libs/subscriber.jar --id <id>

  This option specifies a unique identity for this subscriber. When
  you use this option, you should also run the subscriber on a fixed
  port by setting the Clock.Subscriber.Endpoints property. For
  example:

  $ subscriber --Clock.Subscriber.Endpoints="tcp -p <port> -h <host>"

  Replace "tcp" with "udp" when using the --datagram option.

java -jar build/libs/subscriber.jar --retryCount <count>

  This option sets the retry count for a subscriber. This option
  should be used in conjunction with the --id option. Setting
  retryCount changes the default subscriber QoS to twoway.

For the publisher:

java -jar build/libs/publisher.jar --oneway

  The publisher sends events as oneway messages. This is the default.

java -jar build/libs/publisher.jar --datagram

  The publisher sends events as datagrams.

java -jar build/libs/publisher.jar --twoway

  The publisher sends events as twoway messages.


Running the demo on several hosts
---------------------------------

You must modify several configuration files in order to run the
publisher, subscriber, and IceStorm service on separate hosts. For
the sake of discussion, let us assume that the processes are running
on the following hosts:

  Host <P>: publisher
  Host <S>: subscriber
  Host <I>: IceStorm

Replace <P>, <S>, and <I> with the appropriate host names or IP
addresses in the steps below:

1. Edit the subscriber's configuration file, config.sub, and change
   the TopicManager.Proxy property to

   TopicManager.Proxy=DemoIceStorm/TopicManager:default -h <I> -p 10000

2. Edit the publisher's configuration file, config.pub, and change
   the TopicManager.Proxy property to

   TopicManager.Proxy=DemoIceStorm/TopicManager:default -h <I> -p 10000

3. Since the default configurations for the subscriber and IceStorm
   endpoints listen only to localhost, you must also modify the Endpoints
   properties to explicitly include the host machine's name or IP
   address.

   In config.sub:

   Clock.Subscriber.Endpoints=tcp -h <S>:udp -h <S>

   In config.service:

   IceStorm.TopicManager.Endpoints=default -h <I> -p 10000
   IceStorm.Publish.Endpoints=tcp -h <I> -p 10001:udp -h <I> -p 10001

If you experience any network delays or errors, edit all of the
configuration files and enable the Ice.Trace.Network property. Running
the processes with this property enabled displays a log of Ice's
network activity and may help you to identify the problem more
quickly.