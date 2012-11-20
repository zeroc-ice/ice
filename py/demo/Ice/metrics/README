This demo illustrates how to retrieve metrics information from Ice
applications.

The Metrics.py script provided with this demo connects to an Ice
application running the Ice administrative facility to provide the
following functionality:

- dump one or all configured metrics views
- disable or enable one or all configured metrics views

To use Metrics.py, you first need to configure your application to run
the administrative facility and setup some metrics views. See the two
sections below for either enabling IceMX with the Ice demos or with
your application.

Once you have configured IceMX with a demo or your application, you
can run Metrics.py to dump the metrics views:

 $ ./Metrics.py --Endpoints="tcp -p 10002 -h localhost" \
       --InstanceName="server" dump

You can also disable or enable views with the "enable" or "disable"
commands:

 $ ./Metrics.py --Endpoints="tcp -p 10002 -h localhost" \
       --InstanceName="server" disable Debug

 $ ./Metrics.py --Endpoints="tcp -p 10002 -h localhost" \
       --InstanceName="server" enable ByParent

The Endpoints and InstanceName properties passed to the Metrics.py
script must match the value of the Ice.Admin.Endpoints and
Ice.Admin.InstanceName properties configured for the application to
monitor.


Enable IceMX with the Ice demos
-------------------------------

The following Ice demos include IceMX support in their configuration
files that is disabled by default:

- Ice/hello
- Ice/latency
- Ice/throughput
- Glacier2/chat
- IceStorm/clock

To enable IceMX, you need to un-comment the following property in the
configuration file of the demo application you wish to monitor:

 Ice.Admin.Endpoints

Once this property is enabled, restart the client or server.


Enable IceMX with your application
----------------------------------

First you need to enable the Ice administrative facility by setting the
following properties:

  Ice.Admin.Endpoints
  Ice.Admin.InstanceName

For example:

  Ice.Admin.Endpoints=tcp -p 10002 -h localhost
  Ice.Admin.InstanceName=server

Note that enabling the Ice administrative facility can have security
implications if your network is not secured. See the Ice manual for
more information on this.

Next, you need to configure metrics views. You can for example use the
following properties to enable two metrics views:

  IceMX.Metrics.Debug.GroupBy=id
  IceMX.Metrics.ByParent.GroupBy=parent

The Debug metrics view provides detailed metrics information while the
ByParent metrics view groups metrics by parent (the parent is usually
either the Ice communicator or object adapter depending on the object
monitored).
