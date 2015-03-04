This directory contains a PHP/HTML application that demonstrates how
a PHP session can register an Ice communicator and use it in
subsequent page requests.

To use the application, a user must first log in by providing any
username and password combination (see login.php). Clicking the
'Login' button causes the application to create a communicator,
establish a Glacier2 session, and redirect the user to the session
page (see session.php). From this page the user can invoke the
'sayHello' operation on a hello server via the Glacier2 session.

NOTE: As explained in the Ice manual, the ability to register a
communicator for use by a PHP session is only useful when the
session's page requests are serviced by the same web server process.
In order to use this demo effectively, you may need to modify your
Web server configuration (e.g., to use a single persistent process).
Non-persistent Web server models (e.g., CGI) cannot be used with this
demo.

Follow these steps to install the demo:

1) Install the Ice extension and run-time files as described in the
   manual. Restart the Web server if necessary.

2) Install the following files in your Web server's document
   directory:

   * login.php
   * session.php
   * Hello.php

   The file Hello.php is generated from Hello.ice when you run make or
   nmake in this directory. Verify that the files have appropriate
   access rights.

3) You may need to edit login.php and session.php so that the scripts
   are able to include the Ice run time files. For example, if you
   installed Ice in C:\Ice, add a call to ini_set as shown below:

   <?php
   ini_set('include_path',
           ini_get('include_path') . PATH_SEPARATOR . 'C:/Ice/php');
   require 'Ice.php';
   ...

4) In a command window on the Web server host, start a Glacier2 router
   using the configuration file provided in this directory:

   % glacier2router --Ice.Config=config.glacier2

5) In a separate command window on the same host, start a hello
   server. You can use a server from any Ice language mapping.

6) Start a Web browser and open the login.php page to begin using the
   demo. Note that the Glacier2 configuration uses a session timeout
   of 30 seconds.

If you want to run the Glacier2 router on a different host than the
Web server, you will need to modify the router's endpoint in
config.glacier2 and login.php. To run the hello server on a different
host, modify the endpoint in session.php.


======================================================================
SELinux Notes
======================================================================

SELinux augments the traditional Unix permissions with a number of
new features. In particular, SELinux can prevent the httpd daemon from
opening network connections and reading files without the proper
SELinux types.

If you suspect that your IcePHP application does not work due to
SELinux restrictions, we recommend that you first try it with SELinux
disabled. As root, run:

# setenforce 0

to disable SELinux until the next reboot of your computer.

If you want to run httpd with IcePHP and SELinux enabled, you must do
the following:

- Allow httpd to open network connections:

  # setsebool httpd_can_network_connect=1 

  (add the -P option to make this setting persistent across reboots)

- Make sure any .php file used by your application can be read by 
  httpd. The enclosing directory also needs to be accessible. For
  example:

  # chcon -R -t httpd_sys_content_t /opt/MyApp

For more information on SELinux in Red Hat Enterprise Linux, refer
to the link below:

  http://www.redhat.com/f/pdf/sec/WHP001USselinux.pdf
