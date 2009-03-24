Introduction
------------

This demo presents an unsigned applet that shows how to make
asynchronous Ice invocations in a graphical application. It also
demonstrates how to configure IceSSL using a resource file as the
keystore.


Preparing the applet
--------------------

The demo includes a start page (hello.html) that you will need to
publish on a web server. This page assumes that the demo applet is
stored in a fully self-contained archive named Hello.jar. In order to
create this fully self-contained JAR file, you must build the applet
with ProGuard in your CLASSPATH. After a successful build, copy
Hello.jar from this subdirectory to the same directory as hello.html
on your web server.

NOTE: If you use ProGuard, you must use version 4.3 or later.

If you did not build the applet with ProGuard in your CLASSPATH, the
Hello.jar archive contains only the applet classes. In this case you
must modify the start page to add Ice.jar to the applet's ARCHIVE
parameter:

  <param name = "archive" value = "Ice.jar, Hello.jar">

Alternatively, you can add ProGuard to your CLASSPATH and rebuild the
applet with the following commands:

  ant clean
  ant


Preparing the Ice server
------------------------

The applet requires a "hello" server. You can use the Java server
from the ../hello directory or a hello server from any other Ice
language mapping. You may need to temporarily relax the firewall
restrictions on your server host to allow the applet to establish
connections to the hello server.


Loading the applet locally
--------------------------

The simplest way to use the applet is to open the start page
(hello.html) as a local file directly from your browser. In this case,
you must start the hello server on the same host and enter "localhost"
or "127.0.0.1" as the server host name in the applet.

Note however that the applet security manager in some versions of Java
may prevent the applet from connecting to the local host. If the
applet reports a SocketException error in its status bar, open the
Java console and review the exception stack trace. The most likely
occurrence is an "access denied" error, which indicates that the
security manager rejected the applet's connection attempt. You can
solve this issue by temporarily granting for the applet to connect to
the local machine. For example, if you are using JRE6 on Windows, open
the following file:

  C:\Program Files\Java\jre6\lib\security\java.policy

In the "grant" section, add the line shown below:

  permission java.net.SocketPermission "localhost:1024-", "connect,resolve";

Restart your browser and try the applet again. If the applet still
cannot establish a connection, try loading it from a web server
instead.


Loading the applet from a web server
------------------------------------

Start a hello server on the web server host. Next, start a web browser
and open the start page (hello.html) on your web server.

Once the applet has started, verify that the name of your web server
host is shown correctly in the "Hostname" field.


Using the applet
----------------

Press the "Hello World!" button. You will notice that the server
prints a "Hello World!" message to the console for each invocation. To
make other types of Ice invocations, select a different mode from the
combobox.

The two sliders allow you to experiment with various timeout settings.
The "Timeout" slider determines how long the Ice run time will wait
for an invocation to complete, while the "Delay" slider forces the
server to delay its response. The value of each slider is shown in
seconds. To force a timeout, select a non-zero timeout and set the
delay to be larger than the timeout. The server prints two "Hello
World!" messages in this case because the Slice operation sayHello is
marked as idempotent, meaning that Ice does not need to follow the
at-most-once retry semantics. See the manual for more information
about retry behavior.
