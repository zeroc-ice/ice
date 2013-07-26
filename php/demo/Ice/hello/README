This directory contains a PHP/HTML implementation of the Ice hello
client.

Follow these steps to install the demo:

1) Install the Ice extension and run-time files as described in the
   manual. Restart the Web server if necessary.

2) Install the following files in your Web server's document
   directory:

   * client.php (or client_ns.php, if using PHP namespaces)
   * Hello.php

   The file Hello.php is generated from Hello.ice when you run make or
   nmake in this directory. Verify that the files have appropriate
   access rights.

3) You may need to edit client.php so that the script is able to
   include the Ice run time files. For example, if you installed Ice
   in C:\Ice, add a call to ini_set as shown below:

   <?php
   ini_set('include_path',
           ini_get('include_path') . PATH_SEPARATOR . 'C:/Ice/php');
   require 'Ice.php';
   ...

4) In a command window on the Web server host, start a hello server.
   You can use a server from any Ice language mapping.

5) Start a Web browser and open the client page to begin using the
   demo. Note that support for secure invocations via SSL are disabled
   by default. If you have configured SSL for the Ice extension, edit
   client.php and set the have_ssl variable to true.


=========================================================================
SELinux Notes
=========================================================================

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
