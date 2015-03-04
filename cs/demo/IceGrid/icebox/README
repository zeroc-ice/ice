Note for Mono:

    For this demo to work properly using Mono on Linux, you must
    configure Linux to recognize Mono binaries. More details are
    provided in the release notes:

    http://doc.zeroc.com/display/Ice/Release+Notes

    Alternatively, if you do not want to reconfigure your kernel, edit
    application.xml and change the <icebox> element to read:

    <icebox id="IceBox" exe="mono" activation="on-demand">
      <option>../../../bin/iceboxnet.exe</option>

    Note that we use "mono" (the Mono interpreter) as the executable
    and pass the path name of the iceboxnet executable as an argument.
    If you are not using an Ice source distribution, you will need to
    change the value of the <option> element to reflect the location
    of the iceboxnet executable in your installation, such as
    /usr/bin/iceboxnet.exe or /opt/Ice-3.5.1/bin/iceboxnet.exe.

This demo shows a very simple IceGrid deployment, with a single IceBox
server hosting a number of IceBox services.

The demo also shows how to use an environment variable in a server
descriptor.

In application.xml, <env>LANG=en</env> defines an environment variable
named "LANG" with value "en". The Hello object implementation HelloI
reads the value of "LANG" and shows the message in the corresponding
language; you can try changing the value to fr, de, es or it.

Note for 64 bit Windows:

In order to run this demo under 64 bit Windows you must add
<ice-install-dir>\bin to your PATH, since that is where iceboxnet.exe
is located.


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

  - Launch the application. Windows users can double-click on the
    file IceGridGUI.jar located in <Ice installation directory>\bin.
    Users with a Linux RPM installation can use a script to start the
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

  - Enter any username and password combination and click Finish to
    log in.

  - Next time you want to connect, the connection will appear in the
    "Saved Connections" dialog, just select it and click Connect.


Running the Client
------------------

$ client.exe

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
(DemoGlacier2) to show how you could administer IceGrid from
"the other side" of a firewall. (In this demo, however, all the
components run on the same system.)

Follow these steps:

 - Connect to the IceGrid registry with icegridadmin or the IceGrid
   Admin graphical utility

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
