This demo shows a very simple IceGrid deployment, with a single
IceBox server hosting a number of IceBox services.

The demo also demonstrates how to use an environment variable in a
server descriptor.

In application.xml, <env>LANG=en</env> defines an environment variable
named "LANG" with value "en". The Hello object implementation HelloI
reads the value of "LANG" and shows the message in the corresponding
language; you can try to update the value to fr, de, es or it.


Setup
-----

Start the IceGrid registry and node:

 $ icegridnode --Ice.Config=config.grid

Deploy the 'HelloSimpsons' application (in file application.xml) with
the IceGrid Admin graphical tool. If you prefer to use the command-
line utility, use:

 $ icegridadmin --Ice.Config=config.grid -e \
    "application add application.xml"


Using the IceGrid GUI
---------------------

We suggest using the graphical IceGrid administration tool with this
demo. Follow these steps to log into the IceGrid registry and view
the application you deployed above:

  - Launch the application. Windows users can use the IceGrid Admin
    icon from the Start menu, OS X users can use the IceGrid Admin
    icon from the Finder Applications folder, and users with a Linux
    RPM installation can use the icegridgui script to start the
    program:

    $ icegridgui

    On other platforms, start it from a command window as follows:

    $ java -jar IceGridGUI.jar

    The location of the JAR file varies by platform; for a Linux RPM
    installation, the file resides in /usr/share/java. On other
    platforms it can be found in <Ice installation directory>/lib.

  - Select Login... from the File menu.

  - In the "Saved Connections" dialog, click "New Connection" to open
    the "New Connection" wizard.

  - Select "Direct Connection" and click Next.

  - Enter "DemoIceGrid" for the IceGrid instance name and click next.

  - In "Endpoint Configuration", select "A hostname and port number?"
    and click Next.

  - Enter "localhost" for the Hostname. You can leave the port empty
    as the demo uses the default port. Click Next.

  - Enter any username and password combination and click Finish
    to log in.

  - Next time you want to connect, the connection will appear in the
    "Saved Connections" dialog, just select it and click Connect.


Windows Note
------------

If you compiled this demo in debug mode on Windows, you need to modify
the deployment so that IceGrid executes the debug version of the
IceBox server (iceboxd.exe) instead of the release version. The steps
below assume you have already deployed the application using the
instructions provided in the "Setup" section.

We recommend using the IceGrid GUI to make this change. Follow the
instructions in "Using the IceGrid GUI" to start the application and
log into the registry.

  - In the Live Deployment tab, open "node1", right-click on "IceBox",
    and select "Open Definition".

  - In the "Server Properties" pane, scroll down to the field "Path to
    Executable" and change it to "iceboxd".

  - Apply your changes, then redeploy the application by selecting
    "Save to Registry" from the File menu.

If you prefer to use the command-line tool instead, open the file
application.xml in this directory, search for the text "icebox", and
change exe="icebox" to exe="iceboxd". Then redeploy the application
using the following command:

 $ icegridadmin --Ice.Config=config.grid -e \
    "application update application.xml"


Running the Client
------------------

In a command window, start the client as shown below:

$ client

The client simply calls 'sayHello' on the replicated 'hello' object.


Stopping and restarting IceBox services
---------------------------------------

You can use the IceGrid GUI or the command-line utility to stop and
restart IceBox services. The commands below show how to manipulate
the 'Lisa' service:

 $ icegridadmin --Ice.Config=config.grid
 >>> service stop IceBox Lisa
 >>> service start IceBox Lisa


Administration through Glacier2
-------------------------------

This demo also includes the configuration for a Glacier2 router
(DemoGlacier2), to show how you could administer IceGrid from
 "the other side" of a firewall. (In this demo, however, all the
components run on the same system.)

Follow these steps:

 - Connect to the IceGrid registry with icegridadmin or the
   IceGrid Admin graphical utility

 - Start the DemoGlacier2 server

 - Reconnect to the IceGrid registry, this time using a Glacier2
   session. For example, using the command-line utility you must
   supply a proxy for the router:

   $ icegridadmin --Ice.Default.Router="DemoGlacier2/router:tcp -h localhost -p 4063"

   In the IceGrid GUI, use the Routed tab of the Login dialog. Change
   the Glacier2 instance name to "DemoGlacier2" and the endpoints to
   "tcp -h localhost -p 4063".

   DemoGlacier2 is configured to use a built-in permissions verifier
   that does not validate passwords, so you can log in using any
   username/password combination.


Vista Note
----------

On Vista-derived operating systems, the IceGrid node may emit the
following warning:

  warning: Unable to lookup the performance counter name:
  Unable to connect to the specified computer or the computer is
  offline. This usually occurs when you do not have sufficient
  privileges.

This issue can be resolved by granting appropriate permissions to the
node's user account. Please refer to the Troubleshooting section of
Appendix H in the Ice manual for more information.
