This demo demonstrates how to configure and deploy IceStorm in a
replicated fashion without using IceGrid.

To run the demo, start the three IceStorm replicas. In three separate
windows:

$ icebox --Ice.Config=config.ib1
$ icebox --Ice.Config=config.ib2
$ icebox --Ice.Config=config.ib3

And then in a separate window:

$ subscriber

In another window:

$ publisher

While the publisher continues to run, "tick" messages should be
displayed in the subscriber window.

With three replicas, any single replica can become unavailable, and
IceStorm will continue to function. However, should two replicas
become unavailable, replication will halt until another instance comes
back online.

For more information on the configuration of HA IceStorm please see
the Ice manual.

Other than configuration, the subscribers and publishers do not have
to do anything special to take advantage of the replication. The
subscriber and publisher are identical to those contained in the clock
demo. See ../clock/README for instructions on running them.
